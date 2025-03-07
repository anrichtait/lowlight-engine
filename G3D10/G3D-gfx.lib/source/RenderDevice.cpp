/**
  \file G3D-gfx.lib/source/RenderDevice.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Image.h"
#include "G3D-base/Log.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/CPUPixelTransferBuffer.h"
#include "G3D-base/Projection.h"
#include "G3D-gfx/glcalls.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/getOpenGLState.h"
#include "G3D-gfx/VertexBuffer.h"
#include "G3D-gfx/AttributeArray.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-gfx/Shader.h"
#include "G3D-gfx/Args.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-gfx/Milestone.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-gfx/AudioDevice.h"

namespace G3D {

thread_local RenderDevice* RenderDevice::current = nullptr;
String RenderDevice::dummyString;

static bool isGLES() {
    // During initialization of RenderState, the OSWindow and RenderDevice themselves
    // may not yet be initialized, so we return a default value of false in that case.
    const OSWindow* window = OSWindow::current();
    return window && (window->settings().api == OSWindow::Settings::API_OPENGL_ES);
}

static bool isEGL() {
    // During initialization of RenderState, the OSWindow and RenderDevice themselves
    // may not yet be initialized, so we return a default value of false in that case.
    const OSWindow* window = OSWindow::current();
    return window && (window->settings().api == OSWindow::Settings::API_OPENGL_EGL);
}

static GLenum toGLBlendFunc(RenderDevice::BlendFunc b) {
    debugAssert(b != RenderDevice::BLEND_CURRENT);
    return GLenum(b);
}


static void _glViewport(double a, double b, double c, double d) {
    glViewport(iRound(a), iRound(b), 
           iRound(a + c) - iRound(a), iRound(b + d) - iRound(b));
}


static GLenum primitiveToGLenum(PrimitiveType primitive) {
    return GLenum(primitive);
}


const String& RenderDevice::getCardDescription() const {
    return cardDescription;
}


void RenderDevice::beginOpenGL() {
    debugAssert(! m_inRawOpenGL);

    beforePrimitive();
    
    m_inRawOpenGL = true;
}


void RenderDevice::endOpenGL() {
    debugAssert(m_inRawOpenGL);
    m_inRawOpenGL = false;
    afterPrimitive();    
}


RenderDevice::RenderDevice() :
    m_window(nullptr), 
    m_deleteWindow(false), 
    m_inRawOpenGL(false), 
    m_inIndexedPrimitive(false) {
    m_initialized = false;
    m_cleanedup = false;
    m_numTextureUnits = 0;
    m_numTextures = 0;
    m_numTextureCoords = 0;
    m_lastTime = System::time();

    memset(currentlyBoundTextures, 0, sizeof(currentlyBoundTextures));

    current = this;
}


RenderDevice::~RenderDevice() {
    debugAssertM(m_cleanedup || ! initialized(), "You deleted an initialized RenderDevice without calling RenderDevice::cleanup()");
}


static void debugCallback(GLenum source,
                    GLenum type,
                    GLuint id,
                    GLenum severity,
                    GLsizei length,
                    const GLchar* message,
                    void* userParam) {
    debugPrintf("-----GL DEBUG INFO-----\nSource: %s\nType: %s\nID: %d\nSeverity: %s\nMessage: %s\n-----------------------\n", 
                GLenumToString(source), GLenumToString(type), id, GLenumToString(severity), message);
}


void RenderDevice::setDebugOutput(bool b) {
    if (b) {
        glEnable(GL_DEBUG_OUTPUT);
        //glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        glDebugMessageCallback((GLDEBUGPROC)G3D::debugCallback, nullptr);
    } else {
        glDisable(GL_DEBUG_OUTPUT);
    }
}


/**
 Used by RenderDevice::init.
 */
static const char* isOk(bool x) {
    return x ? "ok" : "UNSUPPORTED";
}

/**
 Used by RenderDevice::init.
 */
static const char* isOk(void* x) {
    // GCC incorrectly claims this function is not called.
    return isOk(x != nullptr);
}


void RenderDevice::init
   (const OSWindow::Settings&      _settings) {

    m_deleteWindow = true;
    init(OSWindow::create(_settings));
}


OSWindow* RenderDevice::window() const {
    return m_window;
}


void RenderDevice::setWindow(OSWindow* window) {
    debugAssert(initialized());
    debugAssert(window);
    debugAssert(window->m_renderDevice == this);

    m_window = window;
}


void RenderDevice::init(OSWindow* window) {    
    debugAssertGLOk();
    debugAssert(! initialized());    
    debugAssert(window);    
    debugAssertM(glGetInteger(GL_PIXEL_PACK_BUFFER_BINDING) == GL_NONE, "GL_PIXEL_PACK_BUFFER unexpectedly bound");
    
    m_swapBuffersAutomatically = true;
    m_swapGLBuffersPending = false;
    m_window = window;

    OSWindow::Settings settings;
    window->getSettings(settings);

    // Fix the GLES-dependent values now that the window is definitely initialized
    m_state = RenderState();
    
    m_beginEndFrame = 0;

    // Under Windows, reset the last error so that our debug box
    // gives the correct results
    #ifdef G3D_WINDOWS
        SetLastError(0);
    #endif
    debugAssertGLOk();
    const int minimumDepthBits    = min(16, settings.depthBits);
    const int desiredDepthBits    = settings.depthBits;

    const int minimumStencilBits  = settings.stencilBits;
    const int desiredStencilBits  = settings.stencilBits;

    // Don't use more texture units than allowed at compile time.
    m_numTextureUnits  = min(GLCaps::numTextureUnits(), (int)MAX_TRACKED_TEXTURE_UNITS);
    m_numTextureCoords = min(GLCaps::numTextureCoords(), (int)MAX_TRACKED_TEXTURE_UNITS);
    m_numTextures      = min(GLCaps::numTextures(), (int)MAX_TRACKED_TEXTURE_IMAGE_UNITS);

    logPrintf("Setting video mode\n");
   
    setVideoMode();
    debugAssertGLOk();
    if (! strcmp((char*)glGetString(GL_RENDERER), "GDI Generic")) {
        logPrintf(
         "\n*********************************************************\n"
           "* WARNING: This computer does not have correctly        *\n"
           "*          installed graphics drivers and is using      *\n"
           "*          the default Microsoft OpenGL implementation. *\n"
           "*          Most graphics capabilities are disabled.  To *\n"
           "*          correct this problem, download and install   *\n"
           "*          the latest drivers for the graphics card.    *\n"
           "*********************************************************\n\n");
    }
    debugAssertGLOk();
    glViewport(0, 0, width(), height());
    
    int depthBits, stencilBits, redBits, greenBits, blueBits, alphaBits;
    // TODO: Abstract into OSWindow.
    // Update when monitor configuration changes?
    //const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    // TODO: Get actual bit depths for alpha depth and stencil
    redBits = 8;//vidMode->redBits;
    blueBits = 8;//vidMode->blueBits;
    greenBits = 8;//vidMode->greenBits;
    alphaBits = 8;
    stencilBits = 8;
    depthBits = 24;    

    bool depthOk   = depthBits >= minimumDepthBits;
    bool stencilOk = stencilBits >= minimumStencilBits;

    cardDescription = GLCaps::renderer();    

    {
        // Test which texture formats are supported by this card
        logLazyPrintf("%d ImageFormats:\n", ImageFormat::CODE_NUM);
        logLazyPrintf("%20s  %s    %s\n", "Format", "Readable", "Writeable");
        
        for (int code = 0; code < ImageFormat::CODE_NUM; ++code) {
            if ((code == ImageFormat::CODE_DEPTH24_STENCIL8) &&
                (GLCaps::enumVendor() == GLCaps::MESA)) {
                // Mesa crashes on this format
                continue;
            }

            const ImageFormat* fmt = ImageFormat::fromCode((ImageFormat::Code)code);
            
            if (fmt) {
                const bool t = GLCaps::supportsTexture(fmt);
                const bool d = GLCaps::supportsTextureDrawBuffer(fmt);
                logLazyPrintf("%20s  %s         %s\n", 
                              fmt->name().c_str(), 
                              t ? "Yes" : "No ", 
                              d ? "Yes" : "No ");
            }
        }
        logLazyPrintf("\n");
    
        OSWindow::Settings actualSettings;
        window->getSettings(actualSettings);

        // This call is here to make GCC realize that isOk is used.
        (void)isOk(false);
        (void)isOk((void*)nullptr);

        logLazyPrintf(
                 "Capability    Minimum   Desired   Received  Ok?\n"
                 "-------------------------------------------------\n"
                 "* RENDER DEVICE \n"
                 "Depth       %4d bits %4d bits %4d bits   %s\n"
                 "Stencil     %4d bits %4d bits %4d bits   %s\n"
                 "Alpha                           %4d bits   %s\n"
                 "Red                             %4d bits   %s\n"
                 "Green                           %4d bits   %s\n"
                 "Blue                            %4d bits   %s\n"
                 "FSAA                      %2d    %2d    %s\n"

                 "Width             %8d pixels           %s\n"
                 "Height            %8d pixels           %s\n"
                 "Mode                 %10s             %s\n\n",

                 minimumDepthBits, desiredDepthBits, depthBits, isOk(depthOk),
                 minimumStencilBits, desiredStencilBits, stencilBits, isOk(stencilOk),

                 alphaBits, "ok",
                 redBits, "ok", 
                 greenBits, "ok", 
                 blueBits, "ok", 

                 settings.msaaSamples, actualSettings.msaaSamples,
                 isOk(settings.msaaSamples == actualSettings.msaaSamples),

                 settings.width, "ok",
                 settings.height, "ok",
                 (settings.fullScreen ? "Fullscreen" : "Windowed"), "ok"
                 );

        String e;
        bool s = GLCaps::supportsG3D10(e);
        logLazyPrintf("This driver does%s support G3D 10:\n%s\n\n",
                      s ? "" : " NOT", e.c_str());
        logPrintf("Done initializing RenderDevice.\n"); 
    }
    m_initialized = true;

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    m_window->m_renderDevice = this;

    // Always bind one constant VAO for OpenGL 3+
    GLuint VAOID;
    glGenVertexArrays(1, &VAOID);
    glBindVertexArray(VAOID);

    debugAssert(m_state.drawFramebuffer && m_state.readFramebuffer);
}


void RenderDevice::describeSystem(String& s) {    
    TextOutput t;
    describeSystem(t);
    t.commitString(s);
}


bool RenderDevice::initialized() const {
    return m_initialized;
}


#ifdef G3D_WINDOWS

HDC RenderDevice::getWindowHDC() const {
    return wglGetCurrentDC();
}

#endif


void RenderDevice::setVideoMode() {
    debugAssertGLOk();

    debugAssertM(m_stateStack.size() == 0, 
                 "Cannot call setVideoMode between pushState and popState");
    debugAssertM(m_beginEndFrame == 0, 
                 "Cannot call setVideoMode between beginFrame and endFrame");

    // Reset all state
    OSWindow::Settings settings;
    m_window->getSettings(settings);

    // Set the refresh rate
#   ifdef G3D_WINDOWS
        if (settings.asynchronous) {
            logLazyPrintf("wglSwapIntervalEXT(0);\n");
            wglSwapIntervalEXT(0);
            debugAssertGLOk();
        } else {
            logLazyPrintf("wglSwapIntervalEXT(1);\n");
            wglSwapIntervalEXT(1);
            debugAssertGLOk();
        }
#   endif

    // Smoothing by default on NVIDIA cards.  On ATI cards
    // there is a bug that causes shaders to generate incorrect
    // results and run at 0 fps, so we can't enable this.
    if (! beginsWith(GLCaps::vendor(), "ATI") && (GLCaps::api() == OSWindow::Settings::API_OPENGL)) {
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        glEnable(GL_LINE_SMOOTH);
        debugAssertGLOk();
    }
    
    // glHint(GL_GENERATE_MIPMAP_HINT_EXT, GL_NICEST);
    if (GLCaps::supports("GL_ARB_multisample")) {
        glEnable(GL_MULTISAMPLE_ARB); 
        debugAssertGLOk();
    }
    
    debugAssertGLOk();
    resetState();

    logPrintf("Done setting initial state.\n");
}


int RenderDevice::width() const {
    return m_state.drawFramebuffer->width();
}


int RenderDevice::height() const {
    return m_state.drawFramebuffer->height();
}

//////////////////////////////////////////////////////////////////////////////////////////


Vector4 RenderDevice::project(const Vector3& v) const {
    return project(Vector4(v, 1));
}


Vector4 RenderDevice::project(const Vector4& v) const {
    //objectToScreenMatrix without invertY
    const Matrix4& M = projectionMatrix() * worldToCameraMatrix() * objectToWorldMatrix();

    Vector4 result = M * v;
    
    const Rect2D& view = viewport();

    // Homogeneous divide
    const double rhw = 1.0 / result.w;

    const float depthRange[2] = {0.0f, 1.0f};

    return Vector4(
        (1.0f + result.x * (float)rhw) * view.width() / 2.0f + view.x0(),
        (1.0f + result.y * (float)rhw) * view.height() / 2.0f + view.y0(),
        (result.z * (float)rhw) * (depthRange[1] - depthRange[0]) + depthRange[0],
        (float)rhw);
}


void RenderDevice::cleanup() {
    debugAssert(initialized());

    logLazyPrintf("Shutting down RenderDevice.\n");

    logPrintf("Freeing all AttributeArray memory\n");

    if (m_deleteWindow) {
        logPrintf("Deleting window.\n");
        VertexBuffer::cleanupAllVertexBuffers();
        delete m_window;
        m_window = nullptr;
    }

    m_cleanedup = true;
}


void RenderDevice::push2D() {
    push2D(viewport());
}


void RenderDevice::push2D(const Rect2D& viewport) {
    push2D(m_state.drawFramebuffer, viewport);
}


void RenderDevice::push2D(const shared_ptr<Framebuffer>& fb) {
    const Rect2D& viewport = 
        (fb && (fb->width() > 0)) ? 
            fb->rect2DBounds() : 
            Rect2D::xywh(0.0f, 0.0f, (float)m_window->width(), (float)m_window->height());
    push2D(fb, viewport);
}


void RenderDevice::push2D(const shared_ptr<Framebuffer>& fb, const Rect2D& viewport) {
    pushState(fb);
    setDepthWrite(false);
    setDepthTest(DEPTH_ALWAYS_PASS);
    setCullFace(CullFace::NONE);
    setViewport(viewport);
    setObjectToWorldMatrix(CoordinateFrame());
    setCameraToWorldMatrix(CoordinateFrame());

    setProjectionMatrix(Matrix4::orthogonalProjection(viewport.x0(), viewport.x0() + viewport.width(), 
                                                      viewport.y0() + viewport.height(), viewport.y0(), -1, 1));

    // Workaround for a bug where setting the draw buffer alone is not
    // enough, or where the order of setting causes problems.
    setFramebuffer(fb);
}


void RenderDevice::pop2D() {
    popState();
}

////////////////////////////////////////////////////////////////////////////////////////////////
RenderDevice::RenderState::RenderState(int width, int height) :

    // WARNING: this must be kept in sync with the initialization code
    // in init();
    viewport(Rect2D::xywh(0.0f, 0.0f, (float)width, (float)height)),
    clip2D(Rect2D::inf()),
    useClip2D(false),

    depthWrite(true),
    colorWrite(true),
    alphaWrite(true),

    depthTest(DEPTH_LEQUAL),
    alphaTest(ALPHA_ALWAYS_PASS),
    alphaReference(0.0),
    
    sRGBConversion(false) {

    drawFramebuffer.reset();
    readFramebuffer.reset();

    srcBlendFuncRGB.resize(16);
    dstBlendFuncRGB.resize(16);
    srcBlendFuncA.resize(16);
    dstBlendFuncA.resize(16);
    blendEqRGB.resize(16);
    blendEqA.resize(16);

    for (int i = 0; i < 16; ++i) {
        srcBlendFuncRGB[i]      = BLEND_ONE;
        dstBlendFuncRGB[i]      = BLEND_ZERO;
        srcBlendFuncA[i]        = BLEND_ONE;
        dstBlendFuncA[i]        = BLEND_ZERO;
        blendEqRGB[i]           = BLENDEQ_ADD;
        blendEqA[i]             = BLENDEQ_ADD;
    }

    // This value may be wrong on the very first initialization if
    // the window itself is not yet initialized. RenderDevice::init()
    // will override it in that case.
    if (isGLES() || isEGL()) {
        drawBuffer                  = DRAW_BACK;
        readBuffer                  = READ_BACK;
    } else {
        drawBuffer                  = DRAW_BACK_LEFT;
        readBuffer                  = READ_BACK_LEFT;
    }

    stencil.stencilTest         = STENCIL_ALWAYS_PASS;
    stencil.stencilReference    = 0;
    stencil.frontStencilFail    = STENCIL_KEEP;
    stencil.frontStencilZFail   = STENCIL_KEEP;
    stencil.frontStencilZPass   = STENCIL_KEEP;
    stencil.backStencilFail     = STENCIL_KEEP;
    stencil.backStencilZFail    = STENCIL_KEEP;
    stencil.backStencilZPass    = STENCIL_KEEP;

    logicOp                     = LOGIC_COPY;

    polygonOffset               = 0;
    pointSize                   = 1;

    renderMode                  = RenderDevice::RENDER_SOLID;

    matrices.objectToWorldMatrix         = CoordinateFrame();
    matrices.cameraToWorldMatrix         = CoordinateFrame();
    matrices.cameraToWorldMatrixInverse  = CoordinateFrame();
    matrices.invertY                     = true;

    stencil.stencilClear        = 0;
    depthClear                  = 1;
    colorClear                  = Color4(0,0,0,1);

    shadeMode                   = SHADE_FLAT;

    // Set projection matrix
    const double aspect = (double)viewport.width() / viewport.height();

    matrices.projectionMatrix = Matrix4::perspectiveProjection(-aspect, aspect, -1, 1, 0.1, 100.0);

    cullFace                    = CullFace::BACK;
    
    lowDepthRange               = 0;
    highDepthRange              = 1;
}


void RenderDevice::resetState() {
    logPrintf("Setting initial rendering state.\n");
    debugAssertGLOk();

    // WARNING: this must be kept in sync with the 
    // RenderState constructor
    m_state = RenderState(m_window->width(), m_window->height());

    m_state.drawFramebuffer = m_window->framebuffer();
    m_state.readFramebuffer = m_window->framebuffer();

    _glViewport(m_state.viewport.x0(), m_state.viewport.y0(), m_state.viewport.width(), m_state.viewport.height());
    debugAssertGLOk();

    glDepthMask(GL_TRUE);
    debugAssertGLOk();
    glClearDepthf(1.0f);
    glColorMask(1,1,1,1);
    glStencilMask((GLuint)~0);
    glDisable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilFunc(GL_ALWAYS, 0, 0xFFFFFFFF);
    glLogicOp(GL_COPY);    
    glDepthFunc(GL_LEQUAL);    
    glEnable(GL_DEPTH_TEST);    
    glDisable(GL_SCISSOR_TEST);    
    glDisable(GL_BLEND);    
    glDisable(GL_POLYGON_OFFSET_FILL);
    debugAssertGLOk();
    glPointSize(1);
    debugAssertGLOk();
    {
        const GLenum buffers[] = { GLenum((isGLES() || isEGL()) ? GL_BACK : GL_BACK_LEFT) };
        glDrawBuffers(1, buffers);
        debugAssertGLOk();
    }
    glReadBuffer((isGLES() || isEGL()) ? GL_BACK : GL_BACK_LEFT);
    debugAssertGLOk();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    debugAssertGLOk();
    glClearStencil(0);
    debugAssertGLOk();
    glClearColor(0,0,0,1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    debugAssertGLOk();
    glDisable(GL_FRAMEBUFFER_SRGB);
    debugAssertGLOk();
    glDepthRangef(0.0f, 1.0f);
    
    debugAssertGLOk();
    debugAssert(m_state.drawFramebuffer && m_state.readFramebuffer);
}


bool RenderDevice::RenderState::Stencil::operator==(const Stencil& other) const {
    return 
        (stencilTest == other.stencilTest) &&
        (stencilReference == other.stencilReference) &&
        (stencilClear == other.stencilClear) &&
        (frontStencilFail == other.frontStencilFail) &&
        (frontStencilZFail == other.frontStencilZFail) &&
        (frontStencilZPass == other.frontStencilZPass) &&
        (backStencilFail == other.backStencilFail) &&
        (backStencilZFail == other.backStencilZFail) &&
        (backStencilZPass == other.backStencilZPass);
}


bool RenderDevice::RenderState::Matrices::operator==(const Matrices& other) const {
    return
        (objectToWorldMatrix == other.objectToWorldMatrix) &&
        (cameraToWorldMatrix == other.cameraToWorldMatrix) &&
        (projectionMatrix == other.projectionMatrix) &&
        (invertY == other.invertY);
}


bool RenderDevice::invertY() const {
    return m_state.matrices.invertY;
}


const Matrix4& RenderDevice::invertYMatrix() const {
    if (invertY()) {
        // "Normal OpenGL"
        static Matrix4 M(1,  0, 0, 0,
                       0, -1, 0, 0,
                       0,  0, 1, 0,
                       0,  0, 0, 1); 
        return M;
    } else {
        // "G3D"
        return Matrix4::identity();
    }
}


void RenderDevice::setState(const RenderState& newState) {
    debugAssertGLOk();
    // The state change checks inside the individual
    // methods will (for the most part) minimize
    // the state changes so we can set all of the
    // new state explicitly.
    
    // Set framebuffer first, since it can affect the viewport
    if (m_state.drawFramebuffer != newState.drawFramebuffer) {
        setDrawFramebuffer(newState.drawFramebuffer);
        debugAssertGLOk();

        // Intentionally corrupt the viewport, forcing renderdevice to
        // reset it below.
        m_state.viewport = Rect2D::xywh(-1, -1, -1, -1);
    }

    if (m_state.readFramebuffer != newState.readFramebuffer) {
        setReadFramebuffer(newState.readFramebuffer);
        debugAssertGLOk();
    }
    if (m_state.readFramebuffer != newState.readFramebuffer) {
        setReadFramebuffer(newState.readFramebuffer);
        debugAssertGLOk();
    }
    setViewport(newState.viewport);

    if (newState.useClip2D) {
        setClip2D(newState.clip2D);
    } else {
        setClip2D(Rect2D::inf());
    }
    
    setDepthWrite(newState.depthWrite);
    setColorWrite(newState.colorWrite);
    setAlphaWrite(newState.alphaWrite);

    setDrawBuffer(newState.drawBuffer);
    setReadBuffer(newState.readBuffer);

    setDepthTest(newState.depthTest);

    if (newState.stencil != m_state.stencil) {
        setStencilConstant(newState.stencil.stencilReference);

        setStencilTest(newState.stencil.stencilTest);

        setStencilOp(
            newState.stencil.frontStencilFail, newState.stencil.frontStencilZFail, newState.stencil.frontStencilZPass,
            newState.stencil.backStencilFail, newState.stencil.backStencilZFail, newState.stencil.backStencilZPass);

        setStencilClearValue(newState.stencil.stencilClear);
    }

    if (m_window->settings().allowAlphaTest) {
        setAlphaTest(newState.alphaTest, newState.alphaReference);
    }

    setDepthClearValue(newState.depthClear);

    setColorClearValue(newState.colorClear);

    for (int i = 0; i < 16; ++i) {
        setBlendFunc(Framebuffer::AttachmentPoint(i + Framebuffer::COLOR0), newState.srcBlendFuncRGB[i], newState.dstBlendFuncRGB[i], newState.blendEqRGB[i], 
                                                                            newState.srcBlendFuncA[i],   newState.dstBlendFuncA[i],   newState.blendEqA[i]);
    }

    setRenderMode(newState.renderMode);
    setPolygonOffset(newState.polygonOffset);
    setPointSize(newState.pointSize);    
    
    if (newState.matrices.cameraToWorldMatrix.toMatrix4().determinant() < 0) {
        glFrontFace(GL_CW);
    } else {
        glFrontFace(GL_CCW);
    }
    

    setCullFace(newState.cullFace);

    setSRGBConversion(newState.sRGBConversion);    

    setDepthRange(newState.lowDepthRange, newState.highDepthRange);
    

    if (m_state.matrices.changed) { //(newState.matrices != m_state.matrices) { 
        if (newState.matrices.cameraToWorldMatrix != m_state.matrices.cameraToWorldMatrix) {
            setCameraToWorldMatrix(newState.matrices.cameraToWorldMatrix);
        }

        if (newState.matrices.objectToWorldMatrix != m_state.matrices.objectToWorldMatrix) {
            setObjectToWorldMatrix(newState.matrices.objectToWorldMatrix);
        }

        setProjectionMatrix(newState.matrices.projectionMatrix);
    }
    
    // Adopt the popped state's deltas relative the state that it replaced.
    m_state.matrices.changed = newState.matrices.changed;

    debugAssertGLOk();
}


void RenderDevice::syncDrawBuffer(bool alreadyBound) {
    if (m_state.drawFramebuffer->isHardwareFramebuffer()) {
        return;
    }

    if (m_state.drawFramebuffer->bind(alreadyBound, Framebuffer::MODE_DRAW)) {
       
        // Apply the bindings from this framebuffer
        const Array<GLenum>& array = m_state.drawFramebuffer->openGLDrawArray();
        if (array.size() > 0) {
            debugAssertM(array.size() <= glGetInteger(GL_MAX_DRAW_BUFFERS),
                         format("This graphics card only supports %d draw buffers.",
                                glGetInteger(GL_MAX_DRAW_BUFFERS)));
            
            glDrawBuffers(array.size(), array.getCArray());
        } else {
            // May be only depth or stencil; don't need a draw buffer.
            
            // Some drivers crash when providing nullptr or an actual
            // zero-element array for a zero-element array, so make a fake
            // array.
            const GLenum noColorBuffers[] = { GL_NONE };
            glDrawBuffers(1, noColorBuffers);
        }
    }
}


RenderDevice::AlphaTest RenderDevice::alphaTest() const {
    return m_state.alphaTest;
}


float RenderDevice::alphaTestReference() const {
    return m_state.alphaReference;
}


void RenderDevice::setAlphaTest(AlphaTest test, float reference) {
    debugAssertM(m_window->settings().allowAlphaTest, "Alpha test has not been allowed for this OpenGL context and window. See OSWindow::Settings::allowAlphaTest");
 
    minStateChange();

    if (test == ALPHA_CURRENT) {
        return;
    }
    
    if ((m_state.alphaTest != test) || (m_state.alphaReference != reference)) {
        minGLStateChange();
        if (test == ALPHA_ALWAYS_PASS) {
            
            glDisable(GL_ALPHA_TEST);

        } else {

            glEnable(GL_ALPHA_TEST);
            switch (test) {
            case ALPHA_LESS:
                glAlphaFunc(GL_LESS, reference);
                break;

            case ALPHA_LEQUAL:
                glAlphaFunc(GL_LEQUAL, reference);
                break;

            case ALPHA_GREATER:
                glAlphaFunc(GL_GREATER, reference);
                break;

            case ALPHA_GEQUAL:
                glAlphaFunc(GL_GEQUAL, reference);
                break;

            case ALPHA_EQUAL:
                glAlphaFunc(GL_EQUAL, reference);
                break;

            case ALPHA_NOTEQUAL:
                glAlphaFunc(GL_NOTEQUAL, reference);
                break;

            case ALPHA_NEVER_PASS:
                glAlphaFunc(GL_NEVER, reference);
                break;

            default:
                debugAssertM(false, "Fell through switch");
            }
        }

        m_state.alphaTest = test;
        m_state.alphaReference = reference;
    }
}
static GLenum toFBOReadBuffer(RenderDevice::ReadBuffer b, const shared_ptr<Framebuffer>& fbo) {

    switch (b) {
    case RenderDevice::READ_FRONT:
    case RenderDevice::READ_BACK:
    case RenderDevice::READ_FRONT_LEFT:
    case RenderDevice::READ_FRONT_RIGHT:
    case RenderDevice::READ_BACK_LEFT:
    case RenderDevice::READ_BACK_RIGHT:
    case RenderDevice::READ_LEFT:
    case RenderDevice::READ_RIGHT:
        if (fbo->has(Framebuffer::COLOR0)) {
            return GL_COLOR_ATTACHMENT0;
        } else {
            return GL_NONE;
        }
        
    default:
        // The specification and various pieces of documentation are
        // unclear on what arguments glReadBuffer may have.
        // http://www.opengl.org/wiki/Framebuffer_Object#Multiple_Render_Targets
        // and the extension specification indicate that COLOR0 is a valid argument
        // even though the OpenGL 4.0 spec does not.
        if (fbo->has(Framebuffer::AttachmentPoint(b))) {
            return GLenum(b);
        } else {
            return GL_NONE;
        }
    }
}


void RenderDevice::syncReadBuffer(bool alreadyBound) {
    if (m_state.readFramebuffer->isHardwareFramebuffer()) {
        return;
    }

    // The return value only tells us if we had to sync attachment points, tells us nothing about
    // the read buffer.
    m_state.readFramebuffer->bind(alreadyBound, Framebuffer::MODE_READ);
    if ((m_state.readFramebuffer->numAttachments() == 1) &&
        (m_state.readFramebuffer->get(Framebuffer::DEPTH))) {
        // OpenGL spec requires that readbuffer be none if there is no color buffer
        glReadBuffer(GL_NONE);
    } else {
        glReadBuffer(toFBOReadBuffer(m_state.readBuffer, m_state.readFramebuffer));
    }
    
}


void RenderDevice::beforePrimitive() {
    debugAssertM(! m_inRawOpenGL, "Cannot make RenderDevice calls while inside beginOpenGL...endOpenGL");
    
    syncDrawBuffer(true);    
    syncReadBuffer(true);
}


void RenderDevice::afterPrimitive() {
}


GLenum RenderDevice::applyWinding(GLenum f) const {
    if (! invertY()) {
        if (f == GL_FRONT) {
            return GL_BACK;
        } else if (f == GL_BACK) {
            return GL_FRONT;
        }
    }

    // Pass all other values (like GL_ALWAYS) through
    return f;
}


void RenderDevice::setRenderMode(RenderMode m) {
    minStateChange();

    if (m == RENDER_CURRENT) {
        return;
    }

    if (m_state.renderMode != m) {
        minGLStateChange();
        m_state.renderMode = m;
        switch (m) {
        case RENDER_SOLID:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;

        case RENDER_WIREFRAME:
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;

        case RENDER_POINTS:
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
            break;

        case RENDER_CURRENT:
            return;
            break;
        }
    }
}


RenderDevice::StencilTest RenderDevice::stencilTest() const {
    return m_state.stencil.stencilTest;
}


RenderDevice::RenderMode RenderDevice::renderMode() const {
    return m_state.renderMode;
}


void RenderDevice::setDrawBuffer(DrawBuffer b) {
    minStateChange();

    if (b == DRAW_CURRENT) {
        return;
    }

    if (m_state.drawFramebuffer->isHardwareFramebuffer()) {
        alwaysAssertM
            (!( (b >= DRAW_COLOR0) && (b <= DRAW_COLOR15)), 
             "Drawing to a color buffer is only supported by application-created framebuffers!");
    }

    if (b != m_state.drawBuffer) {
        minGLStateChange();
        m_state.drawBuffer = b;
        if (m_state.drawFramebuffer->isHardwareFramebuffer()) {
            // Only update the GL state if there is no framebuffer bound.
            glDrawBuffer(GLenum(m_state.drawBuffer));
        }
    }
}


void RenderDevice::setReadBuffer(ReadBuffer b) {
    minStateChange();

    if (b == READ_CURRENT) {
        return;
    }

    if (b != m_state.readBuffer) {
        minGLStateChange();
        m_state.readBuffer = b;
        if (! m_state.readFramebuffer->isHardwareFramebuffer()) {
            glReadBuffer(toFBOReadBuffer(m_state.readBuffer, m_state.readFramebuffer));
        } else {
            glReadBuffer(GLenum(m_state.readBuffer));
        }
    }
}


void RenderDevice::forceSetCullFace(CullFace f) {
    minGLStateChange();
    if (f == CullFace::NONE) {
        glDisable(GL_CULL_FACE);
    } else {
        glEnable(GL_CULL_FACE);
        glCullFace(applyWinding(GLenum(f.value)));
    }

    m_state.cullFace = f;
}


void RenderDevice::setCullFace(CullFace f) {
    minStateChange();
    if ((f != m_state.cullFace) && (f != CullFace::CURRENT)) {
        forceSetCullFace(f);
    }
}


void RenderDevice::setSRGBConversion(bool b) {    
    minStateChange();
    if (m_state.sRGBConversion != b) {
        m_state.sRGBConversion = b;
        minGLStateChange();
        if (b) {
            glEnable(GL_FRAMEBUFFER_SRGB);
        } else {
            glDisable(GL_FRAMEBUFFER_SRGB);
        }
    }
}


void RenderDevice::pushState() {
    debugAssert(m_state.drawFramebuffer && m_state.readFramebuffer);
    m_stateStack.push(m_state);

    // Record that that the lights and matrices are unchanged since the previous state.
    // This allows popState to restore the lighting environment efficiently.

    m_state.matrices.changed = false;

    m_stats.pushStates += 1;
}


void RenderDevice::popState() {    
    debugAssertM(m_stateStack.size() > 0, "More calls to RenderDevice::pushState() than RenderDevice::popState().");
    setState(m_stateStack.last());
    m_stateStack.popDiscard();
}


void RenderDevice::clearFramebuffer(bool clearColor, bool clearDepthAndStencil) {
    const shared_ptr<Framebuffer>& drawBuf = drawFramebuffer();
    if (notNull(drawBuf)) {
        if (clearColor) {
            for (int i = Framebuffer::COLOR0; i < Framebuffer::DEPTH; ++i) {
                if (drawBuf->has(Framebuffer::AttachmentPoint(i))) {
                    const shared_ptr<Texture>& attachedTexture = drawBuf->get(Framebuffer::AttachmentPoint(i))->texture();
                    if (notNull(attachedTexture)) {
                        const Color4& c = drawBuf->getClearValue(Framebuffer::AttachmentPoint(i)) * attachedTexture->encoding().writeMultiplyFirst() + attachedTexture->encoding().writeAddSecond();
                        glClearBufferfv(GL_COLOR, i - Framebuffer::COLOR0, (GLfloat*)&c);
                    }
                }
            } 
        } // color

        if (clearDepthAndStencil) {
            {
                const shared_ptr<Texture>& attachedTexture = drawBuf->get(Framebuffer::DEPTH)->texture();
                if (notNull(attachedTexture)) {
                    const Color4& c = drawBuf->getClearValue(Framebuffer::DEPTH) * attachedTexture->encoding().writeMultiplyFirst() + attachedTexture->encoding().writeAddSecond();
                    glClearBufferfv(GL_DEPTH, 0, (GLfloat*)&c);
                }
            }

            {
                const shared_ptr<Texture>& attachedTexture = drawBuf->get(Framebuffer::STENCIL)->texture();
                if (notNull(attachedTexture)) {
                    const Color4& c = drawBuf->getClearValue(Framebuffer::STENCIL) * attachedTexture->encoding().writeMultiplyFirst() + attachedTexture->encoding().writeAddSecond();
                    const int i = int(c.r); 
                    glClearBufferiv(GL_DEPTH_STENCIL, 0, &i);
                }
            }

            {
                const shared_ptr<Texture>& attachedTexture = drawBuf->get(Framebuffer::DEPTH_AND_STENCIL)->texture();
                if (notNull(attachedTexture)) {
                    const Color4& c = drawBuf->getClearValue(Framebuffer::DEPTH_AND_STENCIL) * attachedTexture->encoding().writeMultiplyFirst() + attachedTexture->encoding().writeAddSecond();
                    glClearBufferfi(GL_DEPTH_STENCIL, 0, c.r, int(c.g));
                }
            }
        }
    } else {
        clear(clearColor, clearDepthAndStencil, clearDepthAndStencil);
    }
}


void RenderDevice::clear(bool clearColor, bool clearDepth, bool clearStencil) {
    
    syncDrawBuffer(true);
    syncReadBuffer(true);

#   ifdef G3D_DEBUG
    {
        String why;
        debugAssertM(currentDrawFramebufferComplete(why), why);
    }
#   endif
    majStateChange();
    majGLStateChange();

    GLint mask = 0;

    bool oldColorWrite = colorWrite();
    if (clearColor) {
        mask |= GL_COLOR_BUFFER_BIT;
        setColorWrite(true);
    }

    bool oldDepthWrite = depthWrite();
    if (clearDepth) {
        mask |= GL_DEPTH_BUFFER_BIT;
        setDepthWrite(true);
    }

    if (clearStencil) {
        mask |= GL_STENCIL_BUFFER_BIT;
        minGLStateChange();
        minStateChange();
    }

    glClear(mask);
    minGLStateChange();
    minStateChange();
    setColorWrite(oldColorWrite);
    setDepthWrite(oldDepthWrite);
}


void RenderDevice::beginFrame() {
    if (m_swapGLBuffersPending) {
        swapBuffers();
    }

    // Because of modal dialogs, this can be higher than 0 but should never be negative or 
    // grow without bound
    debugAssertM((m_beginEndFrame >= 0) && (m_beginEndFrame < 10), "Mismatched calls to beginFrame/endFrame");
    ++m_beginEndFrame;
}


void RenderDevice::swapBuffers() {
    debugAssert(m_state.drawFramebuffer && m_state.readFramebuffer);
    double now = System::time();
    double dt = now - m_lastTime;
    if (dt <= 0) {
        dt = 0.0001;
    }

    m_stats.frameDuration = float(dt);
    m_stats.frameRate = float(1.0 / dt);
    m_stats.triangleRate = float(m_stats.triangles / dt);

    {
        // small inter-frame time: A (interpolation parameter) is small
        // large inter-frame time: A is big
        double A = clamp(dt * 0.6, 0.001, 1.0);
        if (abs(m_stats.smoothFrameRate - m_stats.frameRate) / max(m_stats.smoothFrameRate, m_stats.frameRate) > 0.18) {
            // There's a huge discrepancy--something major just changed in the way we're rendering
            // so we should jump to the new value.
            A = 1.0;
        }
    
        m_stats.smoothFrameRate     = lerp(m_stats.smoothFrameRate, m_stats.frameRate, (float)A);
        m_stats.smoothTriangleRate  = lerp(m_stats.smoothTriangleRate, m_stats.triangleRate, A);
        m_stats.smoothTriangles     = lerp(m_stats.smoothTriangles, m_stats.triangles, A);
    }

    if ((m_stats.smoothFrameRate == finf()) || (isNaN(m_stats.smoothFrameRate))) {
        m_stats.smoothFrameRate = 1000000;
    } else if (m_stats.smoothFrameRate < 0) {
        m_stats.smoothFrameRate = 0;
    }

    if ((m_stats.smoothTriangleRate == finf()) || isNaN(m_stats.smoothTriangleRate)) {
        m_stats.smoothTriangleRate = 1e20;
    } else if (m_stats.smoothTriangleRate < 0) {
        m_stats.smoothTriangleRate = 0;
    }

    if ((m_stats.smoothTriangles == finf()) || isNaN(m_stats.smoothTriangles)) {
        m_stats.smoothTriangles = 1e20;
    } else if (m_stats.smoothTriangles < 0) {
        m_stats.smoothTriangles = 0;
    }

    m_lastTime = now;
    m_previousFrameStats = m_stats;
    m_stats.reset();

#   ifndef G3D_NO_FMOD
        if (notNull(AudioDevice::instance)) { AudioDevice::instance->update(); }
#   endif

    // Process the pending swap buffers call
    m_swapTimer.tick();

    m_window->swapGLBuffers();
    m_swapTimer.tock();
    m_swapGLBuffersPending = false;
}


void RenderDevice::setSwapBuffersAutomatically(bool b) {
    if (b == m_swapBuffersAutomatically) {
        // Setting to current state; nothing to do.
        return;
    }

    if (m_swapGLBuffersPending) {
        swapBuffers();
    }

    m_swapBuffersAutomatically = b;
}


void RenderDevice::endFrame() {
    --m_beginEndFrame;
    VertexBuffer::resetCacheMarkers();
    

    // Because of modal dialogs, this can be higher than 0 but should never be negative or 
    // grow without bound
    debugAssertM(m_beginEndFrame >= 0 && m_beginEndFrame < 10, "Mismatched calls to beginFrame/endFrame");

    // Schedule a swap buffer iff we are handling them automatically.
    m_swapGLBuffersPending = m_swapBuffersAutomatically;

    debugAssertM((m_beginEndFrame > 0) || (m_stateStack.size() == 0), "Missing RenderDevice::popState or RenderDevice::pop2D.");
}


bool RenderDevice::alphaWrite() const {
    return m_state.alphaWrite;
}


bool RenderDevice::depthWrite() const {
    return m_state.depthWrite;
}


bool RenderDevice::colorWrite() const {
    return m_state.colorWrite;
}


void RenderDevice::setStencilClearValue(int s) {
    
    minStateChange();
    if (m_state.stencil.stencilClear != s) {
        minGLStateChange();
        glClearStencil(s);
        m_state.stencil.stencilClear = s;
    }
}


void RenderDevice::setDepthClearValue(float d) {
    minStateChange();
    
    if (m_state.depthClear != d) {
        minGLStateChange();
        glClearDepth(d);
        m_state.depthClear = d;
    }
}


void RenderDevice::setColorClearValue(const Color4& c) {
    
    minStateChange();
    if (m_state.colorClear != c) {
        minGLStateChange();
        glClearColor(c.r, c.g, c.b, c.a);
        m_state.colorClear = c;
    }
}


void RenderDevice::setViewport(const Rect2D& v) {
    minStateChange();
    if (m_state.viewport != v) {
        forceSetViewport(v);
    }
}


void RenderDevice::forceSetViewport(const Rect2D& v) {
    // Flip to OpenGL y-axis
    float x = v.x0();
    float y;
    if (invertY()) {
        y = height() - v.y1();
    } else { 
        y = v.y0();
    }
    _glViewport(x, y, v.width(), v.height());
    m_state.viewport = v;
    minGLStateChange();
}


void RenderDevice::setClip2D(const Rect2D& clip) {
    minStateChange();

    if (clip.isFinite() || clip.isEmpty()) {
        m_state.clip2D = clip;

        const Rect2D& r = ! clip.isEmpty() ? clip : Rect2D::xywh(0,0,0,0);

        const int clipX0 = iFloor(r.x0());
        const int clipY0 = iFloor(r.y0());
        const int clipX1 = iCeil(r.x1());
        const int clipY1 = iCeil(r.y1());

        int y = 0;
        if (invertY()) {
            y = height() - clipY1;
        } else {
            y = clipY0;
        }
        glScissor(clipX0, y, clipX1 - clipX0, clipY1 - clipY0);

        if (clip.area() == 0) {
            minGLStateChange();
            // On some graphics cards a clip region that is zero area without being (0,0,0,0) 
            // fails to actually clip everything.
            glScissor(0,0,0,0);
            glEnable(GL_SCISSOR_TEST);
        }

        // Enable scissor test itself if not 
        // just adjusting the clip region
        if (! m_state.useClip2D) {
            glEnable(GL_SCISSOR_TEST);
            minStateChange();
            minGLStateChange();
            m_state.useClip2D = true;
        }
    } else if (m_state.useClip2D) {
        // disable scissor test if not already disabled
        minGLStateChange();
        glDisable(GL_SCISSOR_TEST);
        m_state.useClip2D = false;
    }
}


Rect2D RenderDevice::clip2D() const {
    if (m_state.useClip2D) {
        return m_state.clip2D;
    } else {
        return m_state.viewport;
    }
}


void RenderDevice::setProjectionAndCameraMatrix(const Projection& P, const CFrame& C) {
    setProjectionMatrix(P);
    setCameraToWorldMatrix(C);
}


const Rect2D& RenderDevice::viewport() const {
    return m_state.viewport;
}


void RenderDevice::pushState(const shared_ptr<Framebuffer>& fb) {
    debugAssert(m_state.drawFramebuffer && m_state.readFramebuffer);
    pushState();

    if (fb) {
        setFramebuffer(fb);

        // When we change framebuffers, we almost certainly don't want to use the old clipping region
        setClip2D(Rect2D::inf());
        setViewport(fb->rect2DBounds());
    }
    debugAssert(m_state.drawFramebuffer && m_state.readFramebuffer);
}


void RenderDevice::setReadFramebuffer(const shared_ptr<Framebuffer>& fbo) {
    debugAssertM(notNull(fbo), "In G3D 10, the framebuffer should never be null");

    if (fbo != m_state.readFramebuffer) {
        majGLStateChange();

        // Set Framebuffer
            m_state.readFramebuffer = fbo;
        if (isNull(fbo) || fbo->isHardwareFramebuffer()) {
            Framebuffer::bindWindowBuffer(Framebuffer::MODE_READ);

            // Restore the buffer that was in use before the framebuffer was attached
            glReadBuffer(GLenum(m_state.readBuffer));
        } else {
            syncReadBuffer(false);
            // The read enables for this framebuffer will be set during beforePrimitive()            
        }
    }
}


void RenderDevice::setDrawFramebuffer(const shared_ptr<Framebuffer>& fbo) {
    debugAssertM(notNull(fbo), "In G3D 10, the framebuffer should never be null");
    if (fbo != m_state.drawFramebuffer) {
        majGLStateChange();

        // Set Framebuffer
        m_state.drawFramebuffer = fbo;
        if (isNull(fbo) || fbo->isHardwareFramebuffer()) {
            Framebuffer::bindWindowBuffer(Framebuffer::MODE_DRAW);

            // Restore the buffer that was in use before the framebuffer was attached
            glDrawBuffers(1, (GLenum*)&m_state.drawBuffer);
            debugAssertGLOk();
        } else {
            syncDrawBuffer(false);
            debugAssertGLOk();
            // The draw enables for this framebuffer will be set during beforePrimitive()            
        }

        const bool newInvertY = m_state.drawFramebuffer->invertY();
        const bool invertYChanged = (m_state.matrices.invertY != newInvertY);
        if (invertYChanged) {
            m_state.matrices.invertY = newInvertY;
            // Force projection matrix change
            forceSetProjectionMatrix(projectionMatrix());

            // TODO: avoid if full-screen
            forceSetViewport(Rect2D(viewport()));

            // TODO: avoid if no-ops
            forceSetCullFace(m_state.cullFace);

            // TODO: avoid if no-ops
            forceSetStencilOp
                (m_state.stencil.frontStencilFail, m_state.stencil.frontStencilZFail, m_state.stencil.frontStencilZPass,
                 m_state.stencil.backStencilFail,  m_state.stencil.backStencilZFail, m_state.stencil.backStencilZPass);
        }
    }
}


void RenderDevice::setDepthTest(DepthTest test) {  
    minStateChange();

    // On ALWAYS_PASS we must force a change because under OpenGL
    // depthWrite and depth test are dependent.
    if ((test == DEPTH_CURRENT) && (test != DEPTH_ALWAYS_PASS)) {
        return;
    }
    
    if ((m_state.depthTest != test) || (test == DEPTH_ALWAYS_PASS)) {
        minGLStateChange();
        if ((test == DEPTH_ALWAYS_PASS) && (m_state.depthWrite == false)) {
            // http://www.opengl.org/sdk/docs/man/xhtml/glDepthFunc.xml
            // "Even if the depth buffer exists and the depth mask is non-zero, the
            // depth buffer is not updated if the depth test is disabled."
            glDisable(GL_DEPTH_TEST);
        } else {
            minStateChange();
            minGLStateChange();
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GLenum(test));
        }

        m_state.depthTest = test;
    }
}

static GLenum toGLEnum(RenderDevice::StencilTest t) {
    debugAssert(t != RenderDevice::STENCIL_CURRENT);
    return GLenum(t);
}


void RenderDevice::_setStencilTest(RenderDevice::StencilTest test, int reference) {

    if (test == RenderDevice::STENCIL_CURRENT) {
        return;
    }

    const GLenum t = toGLEnum(test);
    glStencilFuncSeparate(GL_FRONT_AND_BACK, t, reference, 0xFFFFFFFF);
    minGLStateChange(1);
}


void RenderDevice::setStencilConstant(int reference) {
    minStateChange();
    
    if (m_state.stencil.stencilReference != reference) {
        m_state.stencil.stencilReference = reference;
        _setStencilTest(m_state.stencil.stencilTest, reference);
        minStateChange();

    }
}


void RenderDevice::setStencilTest(StencilTest test) {
    minStateChange();

    if (test == STENCIL_CURRENT) {
        return;
    }

    

    if (m_state.stencil.stencilTest != test) {
        glEnable(GL_STENCIL_TEST);

        if (test == STENCIL_ALWAYS_PASS) {

            // Can't actually disable if the stencil op is using the test as well
            // due to an OpenGL limitation.
            if ((m_state.stencil.frontStencilFail   == STENCIL_KEEP) &&
                (m_state.stencil.frontStencilZFail  == STENCIL_KEEP) &&
                (m_state.stencil.frontStencilZPass  == STENCIL_KEEP) &&
                (! GLCaps::supports_GL_EXT_stencil_two_side() ||
                 ((m_state.stencil.backStencilFail  == STENCIL_KEEP) &&
                  (m_state.stencil.backStencilZFail == STENCIL_KEEP) &&
                  (m_state.stencil.backStencilZPass == STENCIL_KEEP)))) {
                minGLStateChange();
                glDisable(GL_STENCIL_TEST);
            }

        } else {

            _setStencilTest(test, m_state.stencil.stencilReference);
        }

        m_state.stencil.stencilTest = test;

    }
}

 void RenderDevice::setLogicOp(const LogicOp op) {
     
     minStateChange();
 
     if (op == LOGICOP_CURRENT) {
         return;
     }

     if (m_state.logicOp != op) {
         minGLStateChange();
         if (op == LOGIC_COPY) {
              ///< Colour index mode. Redundant?
             glDisable(GL_LOGIC_OP);
             glDisable(GL_COLOR_LOGIC_OP);
         } else {
             ///< Colour index mode. Redundant?
             glEnable(GL_LOGIC_OP); 
             glEnable(GL_COLOR_LOGIC_OP);
             glLogicOp(op);
         }
         m_state.logicOp = op;
     }
}


GLint RenderDevice::toGLStencilOp(RenderDevice::StencilOp op) const {
    debugAssert(op != STENCILOP_CURRENT);
    switch (op) {
    case RenderDevice::STENCIL_INCR_WRAP:
        if (GLCaps::supports_GL_EXT_stencil_wrap()) {
            return GL_INCR_WRAP_EXT;
        } else {
            return GL_INCR;
        }

    case RenderDevice::STENCIL_DECR_WRAP:
        if (GLCaps::supports_GL_EXT_stencil_wrap()) {
            return GL_DECR_WRAP_EXT;
        } else {
            return GL_DECR;
        }
    default:
        return GLenum(op);
    }
}


void RenderDevice::copyTextureFromScreen(const shared_ptr<Texture>& texture, const Rect2D& rect, const ImageFormat* format, int mipLevel, CubeFace face) {  
    if (format == nullptr) {
        format = texture->format();
    }

    int y = 0;
    if (invertY()) {
        y = iRound(viewport().height() - rect.y1());
    } else {
        y = iRound(rect.y0());
    }

    texture->copyFromScreen(Rect2D::xywh(rect.x0(), (float)y, rect.width(), rect.height()), format);
}




void RenderDevice::forceSetStencilOp(
    StencilOp                       frontStencilFail,
    StencilOp                       frontZFail,
    StencilOp                       frontZPass,
    StencilOp                       backStencilFail,
    StencilOp                       backZFail,
    StencilOp                       backZPass) {

    if (! invertY()) {
        std::swap(frontStencilFail, backStencilFail);
        std::swap(frontZFail, backZFail);
        std::swap(frontZPass, backZPass);
    }

    minGLStateChange(2);
    glStencilOpSeparate(GL_FRONT, 
        toGLStencilOp(frontStencilFail),
        toGLStencilOp(frontZFail),
        toGLStencilOp(frontZPass));

    glStencilOpSeparate(GL_BACK, 
        toGLStencilOp(backStencilFail),
        toGLStencilOp(backZFail),
        toGLStencilOp(backZPass));

    
}

void RenderDevice::setStencilOp(
    StencilOp                       frontStencilFail,
    StencilOp                       frontZFail,
    StencilOp                       frontZPass,
    StencilOp                       backStencilFail,
    StencilOp                       backZFail,
    StencilOp                       backZPass) {

    minStateChange();

    if (frontStencilFail == STENCILOP_CURRENT) {
        frontStencilFail = m_state.stencil.frontStencilFail;
    }
    
    if (frontZFail == STENCILOP_CURRENT) {
        frontZFail = m_state.stencil.frontStencilZFail;
    }
    
    if (frontZPass == STENCILOP_CURRENT) {
        frontZPass = m_state.stencil.frontStencilZPass;
    }

    if (backStencilFail == STENCILOP_CURRENT) {
        backStencilFail = m_state.stencil.backStencilFail;
    }
    
    if (backZFail == STENCILOP_CURRENT) {
        backZFail = m_state.stencil.backStencilZFail;
    }
    
    if (backZPass == STENCILOP_CURRENT) {
        backZPass = m_state.stencil.backStencilZPass;
    }
    
    if ((frontStencilFail  != m_state.stencil.frontStencilFail) ||
        (frontZFail        != m_state.stencil.frontStencilZFail) ||
        (frontZPass        != m_state.stencil.frontStencilZPass) || 
        (backStencilFail  != m_state.stencil.backStencilFail) ||
        (backZFail        != m_state.stencil.backStencilZFail) ||
        (backZPass        != m_state.stencil.backStencilZPass)) { 

        forceSetStencilOp(frontStencilFail, frontZFail, frontZPass,
                 backStencilFail, backZFail, backZPass);


        // Need to manage the mask as well

        if ((frontStencilFail  == STENCIL_KEEP) &&
            (frontZPass        == STENCIL_KEEP) && 
            (frontZFail        == STENCIL_KEEP) &&
            (backStencilFail  == STENCIL_KEEP) &&
            (backZPass        == STENCIL_KEEP) &&
            (backZFail        == STENCIL_KEEP)) {

            // Turn off writing.  May need to turn off the stencil test.
            if (m_state.stencil.stencilTest == STENCIL_ALWAYS_PASS) {
                // Test doesn't need to be on
                glDisable(GL_STENCIL_TEST);
            }

        } else {
#           ifdef G3D_DEBUG
            {
                int stencilBits = 0;

                if (isNull(drawFramebuffer())) {
                    stencilBits = glGetInteger(GL_STENCIL_BITS);
                } else {
                    stencilBits = drawFramebuffer()->stencilBits();
                }
                debugAssertM(stencilBits > 0,
                    "Allocate nonzero OSWindow.stencilBits in GApp constructor or RenderDevice::init before using the stencil buffer.");
            }
#           endif
            // Turn on writing.  We also need to turn on the
            // stencil test in this case.

            if (m_state.stencil.stencilTest == STENCIL_ALWAYS_PASS) {
                // Test is not already on
                glEnable(GL_STENCIL_TEST);
                _setStencilTest(m_state.stencil.stencilTest, m_state.stencil.stencilReference);
            }
        }

        m_state.stencil.frontStencilFail  = frontStencilFail;
        m_state.stencil.frontStencilZFail = frontZFail;
        m_state.stencil.frontStencilZPass = frontZPass;
        m_state.stencil.backStencilFail   = backStencilFail;
        m_state.stencil.backStencilZFail  = backZFail;
        m_state.stencil.backStencilZPass  = backZPass;
    }
}


void RenderDevice::setStencilOp(
    StencilOp           fail,
    StencilOp           zfail,
    StencilOp           zpass) {
    

    setStencilOp(fail, zfail, zpass, fail, zfail, zpass);
}


static GLenum toGLBlendEq(RenderDevice::BlendEq e) {
    switch (e) {
    case RenderDevice::BLENDEQ_MIN:
        debugAssert(GLCaps::supports("GL_EXT_blend_minmax"));
        return GL_MIN;

    case RenderDevice::BLENDEQ_MAX:
        debugAssert(GLCaps::supports("GL_EXT_blend_minmax"));
        return GL_MAX;

    case RenderDevice::BLENDEQ_ADD:
        return GL_FUNC_ADD;

    case RenderDevice::BLENDEQ_SUBTRACT:
        debugAssert(GLCaps::supports("GL_EXT_blend_subtract"));
        return GL_FUNC_SUBTRACT;

    case RenderDevice::BLENDEQ_REVERSE_SUBTRACT:
        debugAssert(GLCaps::supports("GL_EXT_blend_subtract"));
        return GL_FUNC_REVERSE_SUBTRACT;

    default:
        debugAssertM(false, "Fell through switch");
        return GL_ZERO;
    }
}

void RenderDevice::setBlendFunc(
    Framebuffer::AttachmentPoint    buf,
    BlendFunc                       srcRGB,
    BlendFunc                       dstRGB,
    BlendEq                         eqRGB,
    BlendFunc                       srcA,
    BlendFunc                       dstA,
    BlendEq                         eqA) {

    
    const int index = buf - Framebuffer::COLOR0;
    debugAssert(index >= 0 && index < 16);

    minStateChange();
    if (srcRGB == BLEND_CURRENT) {
        srcRGB = m_state.srcBlendFuncRGB[index];
    }
    
    if (dstRGB == BLEND_CURRENT) {
        dstRGB = m_state.dstBlendFuncRGB[index];
    }

    if (eqRGB == BLENDEQ_CURRENT) {
        eqRGB = m_state.blendEqRGB[index];
    }

    if (srcA   == BLEND_SAME_AS_RGB) {
        srcA   = srcRGB;
    } else if (srcA == BLEND_CURRENT) {
        srcA   = m_state.srcBlendFuncA[index];
    }

    if (dstA   == BLEND_SAME_AS_RGB) {
        dstA   = dstRGB;
    } else if (dstA == BLEND_CURRENT) {
        dstA   = m_state.dstBlendFuncA[index];
    }

    if (eqA == BLENDEQ_CURRENT) {
        eqA = m_state.blendEqA[index];
    } else if (eqA == BLENDEQ_SAME_AS_RGB) {
        eqA = eqRGB;
    }

    
    if ((m_state.dstBlendFuncRGB[index] != dstRGB) ||
        (m_state.srcBlendFuncRGB[index] != srcRGB) ||
        (m_state.blendEqRGB[index]      != eqRGB) ||
        (m_state.blendEqA[index]        != eqA) ||
        (m_state.dstBlendFuncA[index]   != dstA) ||
        (m_state.srcBlendFuncA[index]   != srcA)) {
        // Something changed

        minGLStateChange();

        if ((srcRGB == BLEND_ONE) && (dstRGB == BLEND_ZERO) && (eqRGB != BLENDEQ_REVERSE_SUBTRACT) &&
            (srcA   == BLEND_ONE) && (dstA   == BLEND_ZERO) && (eqA   != BLENDEQ_REVERSE_SUBTRACT)) {
            // The destination weight is always zero
            glDisablei(GL_BLEND, index);
        } else {
            glEnablei(GL_BLEND, index);
#           if defined(G3D_OSX) && 0
            {
                // TODO: Is this really needed? Do we need to enable some extension to get support on GL 1.3, such as 
                // http://www.opengl.org/registry/specs/ARB/draw_buffers_blend.txt?
                alwaysAssertM((srcRGB == srcA) && (dstRGB == dstA), "OSX does not support separate blend functions for RGB and A");
                alwaysAssertM(eqRGB == eqA, "OSX does not support separate blend equations for RGB and A");
                glBlendFunc(toGLBlendFunc(srcRGB), toGLBlendFunc(dstRGB));                                            
                glBlendEquation(toGLBlendEq(eqRGB));
            }
#           else
            {
                glBlendFuncSeparatei(index, toGLBlendFunc(srcRGB), toGLBlendFunc(dstRGB), 
                                            toGLBlendFunc(srcA),   toGLBlendFunc(dstA));
                glBlendEquationSeparatei(index, toGLBlendEq(eqRGB), toGLBlendEq(eqA));
            }
#           endif
            
        }
        m_state.dstBlendFuncRGB[index] = dstRGB;
        m_state.srcBlendFuncRGB[index] = srcRGB;
        m_state.dstBlendFuncA[index]   = dstA;
        m_state.srcBlendFuncA[index]   = srcA;
        m_state.blendEqRGB[index]      = eqRGB;
        m_state.blendEqA[index]        = eqA;
    }
}

void RenderDevice::setBlendFunc(
    BlendFunc src,
    BlendFunc dst,
    BlendEq   eqRGB,
    BlendEq   eqA,
    Framebuffer::AttachmentPoint buf) {
        setBlendFunc(buf, src, dst, eqRGB, BLEND_SAME_AS_RGB, BLEND_SAME_AS_RGB, eqA);
}


void RenderDevice::setPointSize(
    float               width) {
    
    minStateChange();
    if (m_state.pointSize != width) {
        glPointSize(width);
        minGLStateChange();
        m_state.pointSize = width;
    }
}


void RenderDevice::setObjectToWorldMatrix(
    const CoordinateFrame& cFrame) {
    
    minStateChange();
    

    // No test to see if it is already equal; this is called frequently and is
    // usually different.
    m_state.matrices.changed = true;
    m_state.matrices.objectToWorldMatrix = cFrame;
}


const CoordinateFrame& RenderDevice::objectToWorldMatrix() const {
    return m_state.matrices.objectToWorldMatrix;
}


void RenderDevice::setCameraToWorldMatrix(
    const CoordinateFrame& cFrame) {

    
    majStateChange();
    
    m_state.matrices.changed = true;
    m_state.matrices.cameraToWorldMatrix = cFrame;
    m_state.matrices.cameraToWorldMatrixInverse = cFrame.inverse();
}


const CoordinateFrame& RenderDevice::cameraToWorldMatrix() const {
    return m_state.matrices.cameraToWorldMatrix;
}

const CoordinateFrame& RenderDevice::worldToCameraMatrix() const {
    return m_state.matrices.cameraToWorldMatrixInverse;
}


Matrix4 RenderDevice::projectionMatrix() const {
    return m_state.matrices.projectionMatrix;
}


CoordinateFrame RenderDevice::modelViewMatrix() const {
    return worldToCameraMatrix() * objectToWorldMatrix();
}


Matrix4 RenderDevice::modelViewProjectionMatrix() const {
    return projectionMatrix() * modelViewMatrix() * invertYMatrix();
}


Matrix4 RenderDevice::objectToScreenMatrix() const {
    return  invertYMatrix() * projectionMatrix() * worldToCameraMatrix() * objectToWorldMatrix();
}

void RenderDevice::forceSetProjectionMatrix(const Matrix4& P) {
    m_state.matrices.projectionMatrix = P;
    m_state.matrices.changed = true;
}


void RenderDevice::setProjectionMatrix(const Matrix4& P) {
    minStateChange();
    if (m_state.matrices.projectionMatrix != P) {
        forceSetProjectionMatrix(P);
    }
}


void RenderDevice::setProjectionMatrix(const Projection& P) {
    Matrix4 M;
    P.getProjectUnitMatrix(viewport(), M);
    setProjectionMatrix(M);
}


const ImageFormat* RenderDevice::colorFormat() const {
    shared_ptr<Framebuffer> fbo = drawFramebuffer();
    if (isNull(fbo)) {
        OSWindow::Settings settings;
        window()->getSettings(settings);
        return settings.colorFormat();
    } else {
        shared_ptr<Framebuffer::Attachment> screen = fbo->get(Framebuffer::COLOR0);
        if (isNull(screen)) {
            return nullptr;
        }
        return screen->format();
    }
}


void RenderDevice::setPolygonOffset(
    float              offset) {
    

    minStateChange();

    if (m_state.polygonOffset != offset) {
        minGLStateChange();
        if (offset != 0) {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glEnable(GL_POLYGON_OFFSET_POINT);
            glPolygonOffset(offset, sign(offset) * 2.0f);
        } else {
            glDisable(GL_POLYGON_OFFSET_POINT);
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_POLYGON_OFFSET_LINE);
        }
        m_state.polygonOffset = offset;
    }
}


void RenderDevice::countTriangles(PrimitiveType primitive, int numVertices) {
    switch (primitive) {
    case PrimitiveType::LINES:
        m_stats.triangles += (numVertices / 2);
        break;

    case PrimitiveType::LINE_STRIP:
        m_stats.triangles += (numVertices - 1);
        break;

    case PrimitiveType::TRIANGLES:
        m_stats.triangles += (numVertices / 3);
        break;

    case PrimitiveType::TRIANGLE_STRIP:
    case PrimitiveType::TRIANGLE_FAN:
        m_stats.triangles += (numVertices - 2);
        break;

    case PrimitiveType::QUADS:
        m_stats.triangles += ((numVertices / 4) * 2);
        break;

    case PrimitiveType::QUAD_STRIP:
        m_stats.triangles += (((numVertices / 2) - 1) * 2);
        break;

    case PrimitiveType::POINTS:
        m_stats.triangles += numVertices;
        break;
    }
}


double RenderDevice::getDepthBufferValue(
    int                     x,
    int                     y) const {

    GLfloat depth = 0;
    

    if (m_state.readFramebuffer) {
        debugAssertM(m_state.readFramebuffer->has(Framebuffer::DEPTH),
            "No depth attachment");
    }

    glReadPixels(x,
             (height() - 1) - y,
                 1, 1,
                 GL_DEPTH_COMPONENT,
             GL_FLOAT,
             &depth);

    debugAssertM(glGetError() != GL_INVALID_OPERATION, 
        "getDepthBufferValue failed, probably because you did not allocate a depth buffer.");

    return depth;
}


shared_ptr<Image> RenderDevice::screenshotPic(bool getAlpha, bool invertY) const {
    const shared_ptr<CPUPixelTransferBuffer>& imageBuffer = CPUPixelTransferBuffer::create(width(), height(), getAlpha ? ImageFormat::RGBA8() : ImageFormat::RGB8());

    {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        debugAssertM(glGetInteger(GL_PIXEL_PACK_BUFFER_BINDING) == 0, "GL_PIXEL_PACK_BUFFER bound during glReadPixels");
        debugAssert(glGetInteger(GL_READ_FRAMEBUFFER_BINDING) == 0);
        glReadPixels(0, 0, width(), height(), imageBuffer->format()->openGLBaseFormat, imageBuffer->format()->openGLDataFormat, imageBuffer->buffer());
        
    } 

    const shared_ptr<Image>& image = Image::fromPixelTransferBuffer(imageBuffer);

    // Flip right side up
    if (invertY) {
        image->flipVertical();
    }

    return image;
}


String RenderDevice::screenshot(const String& filepath) const {
    const String filename(FilePath::concat(filepath, generateFilenameBase("", "_" + System::appName()) + ".jpg"));

    const shared_ptr<Image> screen(screenshotPic());
    if (screen) {
        screen->save(filename);
    }

    return filename;
}


void RenderDevice::beginIndexedPrimitives() {
    
    debugAssert(! m_inIndexedPrimitive);

    m_inIndexedPrimitive = true;
}


void RenderDevice::endIndexedPrimitives() {
    
    debugAssert(m_inIndexedPrimitive);

    // Allow garbage collection of VARs
    m_tempVAR.fastClear();
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    m_inIndexedPrimitive = false;
    m_currentVertexBuffer.reset();
}


void RenderDevice::setVARAreaFromVAR(const class AttributeArray& v) {
    debugAssert(m_inIndexedPrimitive);
    
    majStateChange();

    if (v.area() != m_currentVertexBuffer) {
        m_currentVertexBuffer = const_cast<AttributeArray&>(v).area();

        // Bind the buffer (for MAIN_MEMORY, we need do nothing)
        glBindBuffer(GL_ARRAY_BUFFER, m_currentVertexBuffer->m_glbuffer);
        
        majGLStateChange();
    }
}


void RenderDevice::unsetVertexAttribArray(unsigned int attribNum) {
    debugAssert(m_inIndexedPrimitive);
    
    majStateChange();

    if (notNull(m_currentVertexBuffer)) {
        m_currentVertexBuffer = shared_ptr<VertexBuffer>();
        // Bind the buffer (for MAIN_MEMORY, we need do nothing)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        majGLStateChange();
    }

    glDisableVertexAttribArray(attribNum);
}

void RenderDevice::setVertexAttribArray(unsigned int attribNum, const class AttributeArray& v) { 
    setVARAreaFromVAR(v);
    v.vertexAttribPointer(attribNum);
}



void RenderDevice::sendSequentialIndices(PrimitiveType primitive, int numVertices, int start) {

    beforePrimitive();

    glDrawArrays(primitiveToGLenum(primitive), start, numVertices);

    countTriangles(primitive, numVertices);
    afterPrimitive();

    minStateChange();
    minGLStateChange();
}


void RenderDevice::sendSequentialIndicesInstanced(PrimitiveType primitive, int numVertices, int numInstances) {
    
    beforePrimitive();
    debugAssertGLOk();
    
    glDrawArraysInstanced(primitiveToGLenum(primitive), 0, numVertices, numInstances);
    debugAssertGLOk();

    countTriangles(primitive, numVertices * numInstances);    
    afterPrimitive();
    debugAssertGLOk();
    
    minStateChange();
    minGLStateChange();
}


void RenderDevice::sendIndices(PrimitiveType primitive, const IndexStream& indexVAR) {
    sendIndices(primitive, indexVAR, 1, false);
}


void RenderDevice::sendIndicesInstanced(PrimitiveType primitive, const IndexStream& indexVAR, int numInstances) {
    if (numInstances == 1) {
        sendIndices(primitive, indexVAR, 1, false);
    } else {
        sendIndices(primitive, indexVAR, numInstances, true);
    }
}


void RenderDevice::sendIndices(PrimitiveType primitive, const IndexStream& indexVAR,
 int numInstances, bool useInstances) {

    String why;
    debugAssertM(currentDrawFramebufferComplete(why), why);

    debugAssertGLOk();         
    if (indexVAR.m_numElements == 0) {
        // There's nothing in this index array, so don't bother rendering.
        return;
    }

    debugAssertM(indexVAR.area(), "Corrupt AttributeArray");

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexVAR.area()->openGLVertexBufferObject());
    debugAssertGLOk();         

    internalSendIndices(primitive, (int)indexVAR.m_elementSize, indexVAR.m_numElements, 
                        indexVAR.pointer(), numInstances, useInstances);
    debugAssertGLOk();
    countTriangles(primitive, indexVAR.m_numElements * numInstances);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    debugAssertGLOk();         
}


void RenderDevice::sendMultidrawIndices(PrimitiveType primitive, const Array<IndexStream>& indexStreams, int numInstances, bool useInstances) {
    if (indexStreams.size() > 0) {
        String why;
        debugAssertM(currentDrawFramebufferComplete(why), why);

        debugAssertGLOk();

        debugAssertM(indexStreams[0].area(), "Corrupt AttributeArray");
        for (int i = 1; i < indexStreams.size(); ++i) {
            debugAssertM(indexStreams[0].area() == indexStreams[i].area(), why);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexStreams[0].area()->openGLVertexBufferObject());
        debugAssertGLOk();
        int numIndices = 0;

        static Array<int> elementCounts;
        static Array<const GLvoid*> startAddresses;

        for (int i = 0; i < indexStreams.size(); ++i) {
            elementCounts.append(indexStreams[i].size());
            startAddresses.append(indexStreams[i].startAddress());
            numIndices += indexStreams[i].size();
        }

        debugAssertGLOk();
        beforePrimitive();
        debugAssertGLOk();
        GLenum type;

        switch (indexStreams[0].elementSize()) {
        case sizeof(uint32) :
            type = GL_UNSIGNED_INT;
            break;

        case sizeof(uint16) :
            type = GL_UNSIGNED_SHORT;
            break;

        case sizeof(uint8) :
            type = GL_UNSIGNED_BYTE;
            break;

        default:
            debugAssertM(false, "Indices must be either 8, 16, or 32-bytes each.");
            type = 0;
        }

        const GLenum p = primitiveToGLenum(primitive);

        debugAssertGLOk();
        glMultiDrawElements(p, elementCounts.getCArray(), type, startAddresses.getCArray(), elementCounts.size());
        debugAssertGLOk();
        

        afterPrimitive();
        debugAssertGLOk();

        debugAssertGLOk();
        countTriangles(primitive, numIndices * numInstances);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        debugAssertGLOk();

        elementCounts.fastClear();
        startAddresses.fastClear();
    }
}


void RenderDevice::sendMultidrawSequentialIndices
   (PrimitiveType     primitive, 
    const Array<int>& indexCounts, 
    const Array<int>& indexOffsets) {
    beforePrimitive();

    debugAssertM(indexOffsets.size() == indexCounts.size(), "indexOffsets and indexCounts must be the same size");
    glMultiDrawArrays(primitiveToGLenum(primitive), indexOffsets.getCArray(), indexCounts.getCArray(), indexOffsets.size());

    int numVertices = 0;
    for (int i : indexCounts) {
        numVertices += i;
    }
    countTriangles(primitive, numVertices);

    afterPrimitive();

    minStateChange();
    minGLStateChange();
}


void RenderDevice::internalSendIndices
(PrimitiveType           primitive,
 int                     indexSize, 
 int                     numIndices, 
 const void*             index,
 int                     numInstances,
 bool                    useInstances) {
    
    debugAssertGLOk();         
    beforePrimitive();
    GLenum i;
    
    switch (indexSize) {
    case sizeof(uint32):
        i = GL_UNSIGNED_INT;
        break;
    
    case sizeof(uint16):
        i = GL_UNSIGNED_SHORT;
        break;
    
    case sizeof(uint8):
        i = GL_UNSIGNED_BYTE;
        break;
    
    default:
        debugAssertM(false, "Indices must be either 8, 16, or 32-bytes each.");
        i = 0;
    }
    
    const GLenum p = primitiveToGLenum(primitive);

    if (useInstances) {
        glDrawElementsInstanced(p, numIndices, i, index, numInstances);
        debugAssertGLOk();         
    } else {
        glDrawElements(p, numIndices, i, index);        
        debugAssertGLOk();
    }

    afterPrimitive();
}


static bool checkFramebuffer(GLenum which, String& whyNot) {
    const GLenum status = static_cast<GLenum>(glCheckFramebufferStatus(which));

    switch(status) {
    case GL_FRAMEBUFFER_COMPLETE:
        return true;

    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        whyNot = "Framebuffer Incomplete: Incomplete Attachment.";
        break;

    case GL_FRAMEBUFFER_UNSUPPORTED:
        whyNot = "Unsupported framebuffer format.";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        whyNot = "Framebuffer Incomplete: Missing attachment.";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        whyNot = "Framebuffer Incomplete: Missing draw buffer.";
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        whyNot = "Framebuffer Incomplete: Missing read buffer.";
        break;

    default:
        whyNot = format("Framebuffer Incomplete: Unknown error. (0x%X)", status);
    }

    return false;    
}


bool RenderDevice::checkDrawFramebuffer(String& whyNot) const {
    return checkFramebuffer(GL_DRAW_FRAMEBUFFER, whyNot);
}


bool RenderDevice::checkReadFramebuffer(String& whyNot) const {
    return checkFramebuffer(GL_READ_FRAMEBUFFER, whyNot);
}


void RenderDevice::issueMemoryBarrier(GLbitfield type) {
    glMemoryBarrier(type);
}

/////////////////////////////////////////////////////////////////////////////////////

RenderDevice::Stats::Stats() {
    smoothTriangles = 0;
    smoothTriangleRate = 0;
    smoothFrameRate = 0;
    reset();
}


void RenderDevice::Stats::reset() {
    minorStateChanges = 0;
    minorOpenGLStateChanges = 0;
    majorStateChanges = 0;
    majorOpenGLStateChanges = 0;
    pushStates = 0;
    primitives = 0;
    triangles = 0;
    swapbuffersTime = 0;
    frameRate = 0;
    frameDuration = 0;
    triangleRate = 0;
}

/////////////////////////////////////////////////////////////////////////////////////


static void var(TextOutput& t, const String& name, const String& val) {
    t.writeSymbols(name,"=");
    t.writeString(val + ";");
    t.writeNewline();
}


static void var(TextOutput& t, const String& name, const bool val) {
    t.writeSymbols(name, "=", val ? "true;" : "false;");
    t.writeNewline();
}


static void var(TextOutput& t, const String& name, const int val) {
    t.writeSymbols(name, "=");
    t.writeNumber(val);
    t.printf(";");
    t.writeNewline();
}


void RenderDevice::describeSystem(TextOutput& t) {

    
    t.writeSymbols("GPU", "=", "{");
    t.writeNewline();
    t.pushIndent();
    {
        var(t, "Chipset", GLCaps::renderer());
        var(t, "Vendor", GLCaps::vendor());
        var(t, "Driver", GLCaps::driverVersion());
        var(t, "OpenGL version", GLCaps::glVersion());
        var(t, "Textures", GLCaps::numTextures());
        var(t, "Texture coordinates", GLCaps::numTextureCoords());
        var(t, "Texture units", GLCaps::numTextureUnits());
        var(t, "GL_MAX_TEXTURE_SIZE", glGetInteger(GL_MAX_TEXTURE_SIZE));
        var(t, "GL_MAX_CUBE_MAP_TEXTURE_SIZE", glGetInteger(GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT));
        if (GLCaps::supports_GL_ARB_framebuffer_object() || GLCaps::supports_GL_EXT_framebuffer_object()) {
            
            var(t, "GL_MAX_COLOR_ATTACHMENTS", glGetInteger(GL_MAX_COLOR_ATTACHMENTS));
            
        } else {
            var(t, "GL_MAX_COLOR_ATTACHMENTS", 0);
        }
    }
    t.popIndent();
    t.writeSymbols("}", ";");
    t.writeNewline();
    t.writeNewline();

    OSWindow* w = window();
    OSWindow::Settings settings;
    w->getSettings(settings);

    t.writeSymbols("Window", "=", "{");
    t.writeNewline();
    t.pushIndent();
        var(t, "API", w->getAPIName());
        var(t, "Version", w->getAPIVersion());
        t.writeNewline();

        var(t, "In focus", w->hasFocus());
        var(t, "Centered", settings.center);
        var(t, "Framed", settings.framed);
        var(t, "Visible", settings.visible);
        var(t, "Resizable", settings.resizable);
        var(t, "Full screen", settings.fullScreen);
        var(t, "Top", settings.y);
        var(t, "Left", settings.x);
        var(t, "Width", settings.width);
        var(t, "Height", settings.height);
        var(t, "Refresh rate", settings.refreshRate);
        t.writeNewline();

        var(t, "Alpha bits", settings.alphaBits);
        var(t, "Red bits", settings.rgbBits);
        var(t, "Green bits", settings.rgbBits);
        var(t, "Blue bits", settings.rgbBits);
        var(t, "Depth bits", settings.depthBits);
        var(t, "Stencil bits", settings.stencilBits);
        var(t, "Asynchronous", settings.asynchronous);
        var(t, "Stereo", settings.stereo);
        var(t, "FSAA samples", settings.msaaSamples);


        t.writeSymbols("GL extensions", "=", "[");
        t.pushIndent();

        Array<String> ext;
    GLCaps::getExtensions(ext);
        String s = ",\n";
        for (int i = 0; i < ext.length(); ++i) {
            if (i == ext.length() - 1) { s = ""; } // skip the last comma
            t.writeSymbol(trimWhitespace(ext[i]) + s);
        }
        t.popIndent();
        t.writeSymbol("];");
        t.writeNewline();

    t.popIndent();
    t.writeSymbols("};");
    t.writeNewline();
    t.writeNewline();
}

void RenderDevice::modifyArgsForRectModeApply(Args& args) {
    // Mutate args to use regular code path 
    const float zCoord = args.getRectZCoord();
    const Rect2D& r = args.getRect();
    const Rect2D& tcRect = args.getTexCoordRect();

    bool useGiantTriangle = m_state.useClip2D && r.contains(m_state.clip2D);
    
    // Allocate the vertex buffers
    if ((! m_rect2DAttributeArrays.vertexArray.valid()) &&
        (! m_rect2DAttributeArrays.texCoordArray.valid())) {
        // The +1000 here works around a problem with update() at the bottom of this function
        const shared_ptr<VertexBuffer>& dataArea = VertexBuffer::create((sizeof(Vector3) + sizeof(Vector2)) * 4 + 1000, VertexBuffer::WRITE_EVERY_FRAME);
        AttributeArray all((sizeof(Vector3) + sizeof(Vector2)) * 4 + 1000, dataArea);
        m_rect2DAttributeArrays.vertexArray   = AttributeArray(Vector3(), 4, all, 0, (int)sizeof(Vector3));
        m_rect2DAttributeArrays.texCoordArray = AttributeArray(Vector2(), 4, all, sizeof(Vector3) * 4, (int)sizeof(Vector2));
    }

    debugAssertGLOk();

    const RectShaderArgs rectShaderArgs(zCoord, r, tcRect, useGiantTriangle);
    if (rectShaderArgs != m_previousRectShaderArgs) {
        m_previousRectShaderArgs = rectShaderArgs;
        Array<Vector3> vertexArray;
        Array<Vector2> texCoordArray;
        if (useGiantTriangle) {

            // The inner rectangle is the Rect2D we are rendering to
            // When the scissor region is equal to it, we can send the large triangle
            // instead. The vertices with asterisks around them are the vertices of said 
            // larger triangle (the 0 is shared with the inner rectangle)

            //  *0*---1--*2*
            //   |    |  /           
            //   |    | /            
            //   |    |/       
            //   3----2
            //   |   /
            //   |  /
            //   | / 
            //   |/
            //  *1*

            // The formula for the two unique vertices of the triangle, in terms of the rectangle vertices are as follows:
            //
            // t_1 = r0 + 2 * (r_3 - r_0) = 2 * r_3 - r_0
            // t_2 = r0 + 2 * (r_1 - r_0) = 2 * r_1 - r_0
            //
            // where t_n is the n-th vertex of the large triangle and r_m is the m-th vertex of the rectangle 
            //


            Vector3 t1(2 * r.corner(3) - r.corner(0), zCoord);
            Vector3 t2(2 * r.corner(1) - r.corner(0), zCoord);

            Vector2 tc1(2 * tcRect.corner(3) - tcRect.corner(0));
            Vector2 tc2(2 * tcRect.corner(1) - tcRect.corner(0));

            // It's fine to not have a full 4 vertices to update, as we use the correct index buffer
            vertexArray.append(Vector3(r.corner(0), zCoord), t1, t2);
            texCoordArray.append(tcRect.corner(0), tc1, tc2);
        } else {
            //            Desired
            //   Rect2D   Array
            //   0----1   0----2
            //   |    |   |  / |  
            //   |    |   | /  |  
            //   |    |   |/   |  
            //   3----2   1----3

            int remap[4] = { 0, 3, 1, 2 };
            for (int i = 0; i < 4; ++i) {
                vertexArray.append(Vector3(r.corner(remap[i]), zCoord));
                texCoordArray.append(tcRect.corner(remap[i]));
            }
        }
        m_rect2DAttributeArrays.vertexArray.update(vertexArray);
        m_rect2DAttributeArrays.texCoordArray.update(texCoordArray);
    }

    args.clearAttributeAndIndexBindings();
    args.setUniform("g3d_ObjectToScreenMatrixTranspose", objectToScreenMatrix().transpose());
    args.setAttributeArray("g3d_Vertex", m_rect2DAttributeArrays.vertexArray);
    args.setAttributeArray("g3d_TexCoord0", m_rect2DAttributeArrays.texCoordArray);
    args.setNumIndices(useGiantTriangle ? 3 : 4);
    args.setPrimitiveType(useGiantTriangle ? PrimitiveType::TRIANGLES : PrimitiveType::TRIANGLE_STRIP);
}


void RenderDevice::apply(const shared_ptr<Shader>& s, Args& args) {
    int maxModifiedTextureUnit = -1;
    args.append(m_state.drawFramebuffer->uniformTable);
    debugAssertGLOk();
    const shared_ptr<Shader::ShaderProgram>& program = s->compileAndBind(args, this, maxModifiedTextureUnit);
    debugAssertGLOk();
    const Shader::DomainType domainType = Shader::domainType(s, args);

    debugAssertGLOk();
    if (domainType == Shader::RECT_MODE) {
        modifyArgsForRectModeApply(args);
    }

    debugAssertGLOk();
    switch (domainType) {
    case Shader::STANDARD_INDEXED_RENDERING_MODE:
    case Shader::MULTIDRAW_INDEXED_RENDERING_MODE:
    case Shader::STANDARD_NONINDEXED_RENDERING_MODE:
    case Shader::MULTIDRAW_NONINDEXED_RENDERING_MODE:
    case Shader::INDIRECT_RENDERING_MODE:
    case Shader::RECT_MODE:
        beginIndexedPrimitives(); {
                   
             s->bindStreamArgs(program, args, this);

             #ifdef G3D_DEBUG
             if (false) {
                 // Validation code. This tends to produce mostly spam on OS X
                 // instead of useful warnings
                 const GLuint glProgramObject = program->glShaderProgramObject();
                 GLint validated;
                 glValidateProgram(glProgramObject);
                 glGetProgramiv(glProgramObject, GL_VALIDATE_STATUS, &validated);
                 debugAssertGLOk();

                 GLint maxLength = 0, length = 0;
                 glGetProgramiv(glProgramObject, GL_INFO_LOG_LENGTH, &maxLength);
                 GLchar* pInfoLog = (GLchar *)malloc(maxLength * sizeof(GLcharARB));
                 glGetProgramInfoLog(glProgramObject, maxLength, &length, pInfoLog);
                 debugAssertGLOk();

                 if (! validated) {
                     debugPrintf("%s\n", pInfoLog);
                     debugAssertM(false, pInfoLog);
                 }
             }
             #endif

             const bool isMeshShader = false;

             if ((domainType == Shader::STANDARD_INDEXED_RENDERING_MODE)) {
                 
                 if (isMeshShader) {
                     // TODO:
                     // Bind element (index) array buffer?
                     // glDrawMeshTasksNV(firstWorkGroupID, numWorkGroups);
                     // unbind buffer?
                     // TODO: update rendered primitive counts
                 } else {
                     sendIndicesInstanced(args.getPrimitiveType(), args.getIndexStream(), args.numInstances());
                 }
    
             } else if (domainType == Shader::STANDARD_NONINDEXED_RENDERING_MODE || (domainType == Shader::RECT_MODE)) {
                  
                 alwaysAssertM(! isMeshShader, "Cannot apply a mesh shader to a non-indexed domain");

                 sendSequentialIndicesInstanced(args.getPrimitiveType(), args.numIndices(), args.numInstances());
                 debugAssertGLOk();

             } else if (domainType == Shader::INDIRECT_RENDERING_MODE) {
                 // TODO: update the primitive counts

                 glBindBuffer(GL_DRAW_INDIRECT_BUFFER, args.indirectBuffer()->glBufferID());
                 if (isMeshShader) {
                     glDrawMeshTasksIndirectNV(args.indirectOffset());
                 } else {
                     glDrawArraysIndirect(args.getPrimitiveType(), (const void*)args.indirectOffset());
                 }
                 glBindBuffer(GL_DRAW_INDIRECT_BUFFER, GL_NONE);

             } else if (domainType == Shader::MULTIDRAW_NONINDEXED_RENDERING_MODE) {

                 alwaysAssertM(! isMeshShader, "Cannot apply a mesh shader to a non-indexed domain");
                 sendMultidrawSequentialIndices(args.getPrimitiveType(), args.indexCountArray(), args.indexOffsetArray());

             } else {

                 if (isMeshShader) {
                     // TODO:
                     // Bind some things like GL_DRAW_INDIRECT_BUFFER that are in sendMultidrawIndices?
                     // glMultiDrawMeshTasksIndirectNV(args.indexStreamArray(), args.numInstances(), stride);
                 } else {
                     debugAssert(domainType == Shader::MULTIDRAW_INDEXED_RENDERING_MODE);
                     sendMultidrawIndices(args.getPrimitiveType(), args.indexStreamArray(), args.numInstances(), args.numInstances() > 0);
                 }
             }

             s->unbindStreamArgs(program, args, this);
         } endIndexedPrimitives();
         break;

    case Shader::STANDARD_COMPUTE_MODE:
        {
            const Vector3int32& gridDim = args.getComputeGridDim();
            const Vector3int32& groupSize = args.computeGroupSize();
            if (groupSize.nonZero()) {
                glDispatchComputeGroupSizeARB(gridDim.x, gridDim.y, gridDim.z,
                                              groupSize.x, groupSize.y, groupSize.z);
            }
            else {
                glDispatchCompute(gridDim.x, gridDim.y, gridDim.z);
            }
        }
        break;

    case Shader::INDIRECT_COMPUTE_MODE:
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, args.indirectBuffer()->glBufferID());
        glDispatchComputeIndirect(args.indirectOffset());
        glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, GL_NONE);
        break;

    default:
        alwaysAssertM(false, "Invalid Shader + Args configuration. Common causes are forgetting to call setRect() or setAttributeArray() and invoking a compute shader with a graphics shader domain (or vice versa).");
    }

    glUseProgram(GL_NONE); 
    if (domainType == Shader::RECT_MODE) { 
        // Put the args back how we found them
        args.clearAttributeAndIndexBindings();
        args.setRect(m_previousRectShaderArgs.vertices, m_previousRectShaderArgs.zCoord, m_previousRectShaderArgs.texCoord);
    }
}

    
} // namespace

