/**
  \file G3D-app.lib/source/GApp.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/Log.h"
#include "G3D-base/PixelTransferBuffer.h"
#include "G3D-base/CPUPixelTransferBuffer.h"
#include "G3D-base/ParseError.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/units.h"
#include "G3D-base/NetworkDevice.h"
#include "G3D-app/AmbientOcclusion.h"
#include "G3D-app/Camera.h"
#include "G3D-app/CameraControlWindow.h"
#include "G3D-app/DebugTextWidget.h"
#include "G3D-app/DefaultRenderer.h"
#include "G3D-app/DepthOfField.h"
#include "G3D-app/MarkerEntity.h"
#include "G3D-app/Draw.h"
#include "G3D-app/FirstPersonManipulator.h"
#include "G3D-app/GApp.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-gfx/OSWindow.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/GuiTextureBox.h"
#include "G3D-app/Light.h"
#include "G3D-gfx/AudioDevice.h"
#include "G3D-app/MotionBlur.h"
#include "G3D-app/SceneEditorWindow.h"
#include "G3D-app/ScreenCapture.h"
#include "G3D-gfx/Shader.h"
#include "G3D-app/Shape.h"
#include "G3D-app/UprightSplineManipulator.h"
#include "G3D-app/UserInput.h"
#include "G3D-app/EmulatedGazeTracker.h"
#include "G3D-gfx/XR.h"
#include "G3D-app/XRWidget.h"
#include <time.h>

// Force discrete GPU on Optimus
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
#ifdef _MSC_VER
extern "C" {
    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

namespace G3D {

GApp* GApp::s_currentGApp = nullptr;

/** Framerate when the app does not have focus.  Should be low, e.g., 4fps */
static const float BACKGROUND_FRAME_RATE = 4.0; // fps

// Forward reference
void initGLG3D(const G3DSpecification& spec = G3DSpecification());


GApp::Settings::Settings() :
    dataDir("<AUTO>"),
    debugFontName("console-small.fnt"),
    logFilename("log.txt"),
    useDeveloperTools(true),
    developerToolsFontName("arial.fnt"),
    developerToolsThemeName("osx-10.7.gtm"),
    writeLicenseFile(true) {
    initGLG3D();
}


GApp::Settings::Settings(int argc, const char* argv[]) {
    *this = Settings();

    argArray.resize(argc);
    for (int i = 0; i < argc; ++i) {
        argArray[i] = argv[i];
    }
}


Vector2int32 GApp::Settings::HDRFramebufferSettings::hdrFramebufferSizeFromDeviceSize(const Vector2int32 osWindowSize) const {
     return Vector2int32(int(float(osWindowSize.x) * sampleRateOneDimension) + depthGuardBandThickness.x * 2,
                         int(float(osWindowSize.y) * sampleRateOneDimension) + depthGuardBandThickness.y * 2);
}


GApp::Settings::RendererSettings::RendererSettings()
    : factory(&(DefaultRenderer::create))
    , deferredShading(false)
    , orderIndependentTransparency(false) {}


GApp::Settings::ScreenCaptureSettings::ScreenCaptureSettings() {}


GApp* GApp::current() {
    return s_currentGApp;
}


void GApp::setCurrent(GApp* gApp) {
    s_currentGApp = gApp;
}


void screenPrintf(const char* fmt ...) {
    va_list argList;
    va_start(argList, fmt);
    if (GApp::current()) {
        GApp::current()->vscreenPrintf(fmt, argList);
    }
    va_end(argList);
}


void GApp::vscreenPrintf
(const char*                 fmt,
 va_list                     argPtr) {
    if (showDebugText) {
        String s = G3D::vformat(fmt, argPtr);
        const Array<String>& newlineSeparatedStrings = stringSplit(s, '\n');

        std::lock_guard<std::mutex> guard(m_debugTextMutex);
        debugText.append(newlineSeparatedStrings);
    }
}


DebugID debugDraw
(const shared_ptr<Shape>& shape,
 float             displayTime,
 const Color4&     solidColor,
 const Color4&     wireColor,
 const CFrame&     frame) {

    if (GApp::current()) {
        debugAssert(shape);
        GApp::DebugShape& s = GApp::current()->debugShapeArray.next();
        s.shape             = shape;
        s.solidColor        = solidColor;
        s.wireColor         = wireColor;
        s.frame             = frame;
        if (displayTime == 0) {
            s.endTime = 0;
        } else {
            s.endTime       = System::time() + displayTime;
        }
        s.id                = GApp::current()->m_lastDebugID++;
        return s.id;
    } else {
        return 0;
    }
}

DebugID debugDraw
(const Box& b,
 float             displayTime,
 const Color4&     solidColor,
 const Color4&     wireColor,
 const CoordinateFrame&     cframe){
     return debugDraw(std::make_shared<BoxShape>(b), displayTime, solidColor, wireColor, cframe);
}

DebugID debugDraw
(const Array<Vector3>&      vertices,
 const Array<int>&          indices,
 float                      displayTime,
 const Color4&              solidColor,
 const Color4&              wireColor,
 const CoordinateFrame&     cframe){
     return debugDraw(std::make_shared<MeshShape>(vertices, indices), displayTime, solidColor, wireColor, cframe);
}

DebugID debugDraw
(const CPUVertexArray& vertices,
 const Array<Tri>&     tris,
 float                 displayTime,
 const Color4&         solidColor,
 const Color4&         wireColor,
 const CoordinateFrame&     cframe){
     return debugDraw(std::make_shared<MeshShape>(vertices, tris), displayTime, solidColor, wireColor, cframe);
}

DebugID debugDraw
(const Sphere&     s,
 float             displayTime,
 const Color4&     solidColor,
 const Color4&     wireColor,
 const CoordinateFrame&     cframe){
     return debugDraw(std::make_shared<SphereShape>(s), displayTime, solidColor, wireColor, cframe);
}

DebugID debugDraw
(const Point3&     p,
 float             displayTime,
 const Color4&     solidColor,
 const Color4&     wireColor,
 const CoordinateFrame&     cframe){
     const Sphere s(p, 0.0007f);
     return debugDraw(std::make_shared<SphereShape>(s), displayTime, solidColor, wireColor, cframe);
}

DebugID debugDraw
(const CoordinateFrame& cf,
 float             displayTime,
 const Color4&     solidColor,
 const Color4&     wireColor,
 const CoordinateFrame&     cframe){
     return debugDraw(std::make_shared<AxesShape>(cf), displayTime, solidColor, wireColor, cframe);
}


DebugID debugDrawLabel
(const Point3& wsPos,
 const Vector3& csOffset,
 const GuiText& text,
 float displayTime,
 float size,
 bool  sizeInPixels,
 const GFont::XAlign xalign,
 const GFont::YAlign yalign) {

    if (notNull(GApp::current())) {
        GApp::DebugLabel& L = GApp::current()->debugLabelArray.next();
        L.text = text;

        L.wsPos = wsPos + GApp::current()->activeCamera()->frame().vectorToWorldSpace(csOffset);
        if (sizeInPixels) {
            const float factor = -GApp::current()->activeCamera()->imagePlanePixelsPerMeter(GApp::current()->renderDevice->viewport());
            const float z = GApp::current()->activeCamera()->frame().pointToObjectSpace(L.wsPos).z;
            L.size  = (size / factor) * (float)abs(z);
        } else {
            L.size = size;
        }
        L.xalign = xalign;
        L.yalign = yalign;

        if (displayTime == 0) {
            L.endTime = 0;
        } else {
            L.endTime = System::time() + displayTime;
        }

        L.id = GApp::current()->m_lastDebugID;
        ++GApp::current()->m_lastDebugID;
        return L.id;
    } else {
        return 0;
    }
}


DebugID debugDrawLabel
(const Point3& wsPos,
 const Vector3& csOffset,
 const String& text,
 const Color3& color,
 float displayTime,
 float size,
 bool  sizeInPixels,
 const GFont::XAlign xalign,
 const GFont::YAlign yalign) {
     const GuiText t(text, nullptr, -1.0f, color);
     return debugDrawLabel(wsPos, csOffset, t, displayTime, size, sizeInPixels, xalign, yalign);
}


/** Attempt to write license file */
static void writeLicense() {
    FILE* f = FileSystem::fopen("g3d-license.txt", "wt");
    if (f != nullptr) {
        fprintf(f, "%s", license().c_str());
        FileSystem::fclose(f);
    }
}


void GApp::initializeOpenGL(RenderDevice* rd, OSWindow* window, bool createWindowIfNull, const Settings& settings) {
    if (notNull(rd)) {
        debugAssertM(notNull(window), "If you pass in your own RenderDevice, then you must also pass in your own OSWindow when creating a GApp.");

        m_hasUserCreatedRenderDevice = true;
        m_hasUserCreatedWindow = true;
        renderDevice = rd;
    } else if (createWindowIfNull) {
        m_hasUserCreatedRenderDevice = false;
        renderDevice = new RenderDevice();
        if (notNull(window)) {
            m_hasUserCreatedWindow = true;
            renderDevice->init(window);
        } else {
            m_hasUserCreatedWindow = false;
            renderDevice->init(settings.window);
        }
    }
    

    if (isNull(renderDevice)) {
        return;
    }

    m_window = renderDevice->window();
    OSWindow::pushGraphicsContext(m_window);
    m_deviceFramebuffer = m_osWindowDeviceFramebuffer = m_window->framebuffer();

    m_widgetManager = WidgetManager::create(m_window);
    userInput = new UserInput(m_window);

    {
        TextOutput t;

        t.writeSymbols("System","=", "{");
        t.pushIndent();
        t.writeNewline();
        System::describeSystem(t);
        if (renderDevice) {
            renderDevice->describeSystem(t);
        }

        NetworkDevice::instance()->describeSystem(t);
        t.writeNewline();
        t.writeSymbol("};");
        t.writeNewline();

        String s;
        t.commitString(s);
        logPrintf("%s\n", s.c_str());
    }
    m_debugCamera  = Camera::create("(Debug Camera)");
    m_activeCamera = m_debugCamera;

    m_activeCameraMarker = MarkerEntity::create("(Active Camera Marker)", nullptr,
        Array<Box>(Box(Point3(-0.11f, -0.11f, -0.11f), Point3(0.11f, 0.11f, 0.11f))), Color3::green());
    m_activeCameraMarker->setShouldBeSaved(false);
    m_activeListener = m_activeCameraMarker;

    debugAssertGLOk();
    debugFont = loadFont(settings.debugFontName);
    debugAssertGLOk();

    const shared_ptr<FirstPersonManipulator>& manip = FirstPersonManipulator::create(userInput);
    manip->onUserInput(userInput);
    manip->setMoveRate(10);
    manip->setPosition(Vector3(0, 0, 4));
    manip->lookAt(Vector3::zero());
    manip->setMouseMode(FirstPersonManipulator::MOUSE_DIRECT_RIGHT_BUTTON);
    manip->setEnabled(true);
    m_debugCamera->setPosition(manip->translation());
    m_debugCamera->lookAt(Vector3::zero());
    setCameraManipulator(manip);
    m_debugController = manip;

    {
        GConsole::Settings settings;
        settings.backgroundColor = Color3::green() * 0.1f;
        console = GConsole::create(debugFont, settings, staticConsoleCallback, this);
        console->setActive(false);
        addWidget(console);
    }

    m_film = Film::create();

    debugAssert(renderDevice->width() == settings.window.width);

    m_osWindowHDRFramebuffer = Framebuffer::create("GApp::m_osWindowHDRFramebuffer");
    m_framebuffer = m_osWindowHDRFramebuffer;

    // The actual buffer allocation code:
    resize(settings.window.width, settings.window.height);

    const shared_ptr<GFont>&    arialFont = GFont::fromFile(System::findDataFile("icon.fnt"));
    const shared_ptr<GuiTheme>& theme     = GuiTheme::fromFile(System::findDataFile("osx-10.7.gtm"), arialFont);

#   ifndef GLEW_EGL
    debugWindow = GuiWindow::create("Control Window", theme,
        Rect2D::xywh(0.0f, 0.0f, (float)settings.window.width, 150.0f), GuiTheme::PANEL_WINDOW_STYLE, GuiWindow::NO_CLOSE);
    debugPane = debugWindow->pane();
    debugWindow->setVisible(false);
    addWidget(debugWindow);

    debugAssertGLOk();
#   endif 

    m_simTime     = 0;
    m_realTime    = 0;
    m_lastWaitTime  = System::time();

    m_depthOfField = DepthOfField::create();
    m_motionBlur   = MotionBlur::create();

    renderDevice->setColorClearValue(Color3(0.1f, 0.5f, 1.0f));

    m_gbufferSpecification.encoding[GBuffer::Field::CS_NORMAL]        = Texture::Encoding(ImageFormat::RGB10A2(), FrameName::CAMERA, 2.0f, -1.0f);
    m_gbufferSpecification.encoding[GBuffer::Field::DEPTH_AND_STENCIL]     = ImageFormat::DEPTH32();
    m_gbufferSpecification.depthEncoding = DepthEncoding::HYPERBOLIC;

    m_renderer = (settings.renderer.factory != nullptr) ? settings.renderer.factory() : DefaultRenderer::create();

    const shared_ptr<DefaultRenderer>& defaultRenderer = dynamic_pointer_cast<DefaultRenderer>(m_renderer);

    if (settings.renderer.deferredShading && notNull(defaultRenderer)) {
        m_gbufferSpecification.encoding[GBuffer::Field::CS_FACE_NORMAL].format = nullptr;
        m_gbufferSpecification.encoding[GBuffer::Field::EMISSIVE]           = 
            Texture::Encoding(ImageFormat::R11G11B10F(), FrameName::NONE, 3.0f, 0.0f);
        m_gbufferSpecification.encoding[GBuffer::Field::LAMBERTIAN]         = ImageFormat::RGB8();
        m_gbufferSpecification.encoding[GBuffer::Field::GLOSSY]             = ImageFormat::RGBA8();

        defaultRenderer->setDeferredShading(true);
    }

    if (notNull(defaultRenderer)) {
        defaultRenderer->setOrderIndependentTransparency(settings.renderer.orderIndependentTransparency);
    }

    const Vector2int32 hdrSize = m_settings.hdrFramebuffer.hdrFramebufferSizeFromDeviceSize(Vector2int32(m_deviceFramebuffer->vector2Bounds()));
    m_gbuffer = m_osWindowGBuffer = GBuffer::create(m_gbufferSpecification);
    m_gbuffer->resize(hdrSize);

    // Share the depth buffer with the forward-rendering pipeline
    debugAssert(m_osWindowHDRFramebuffer->width() == m_gbuffer->width());
    m_osWindowHDRFramebuffer->set(Framebuffer::DEPTH, m_gbuffer->texture(GBuffer::Field::DEPTH_AND_STENCIL));
    m_ambientOcclusion = AmbientOcclusion::create();

    // This program renders to texture for most 3D rendering, so it can
    // explicitly delay calling swapBuffers until the Film::exposeAndRender call,
    // since that is the first call that actually affects the back buffer.  This
    // reduces frame tearing without forcing vsync on.
    renderDevice->setSwapBuffersAutomatically(false);

    m_debugTextWidget = DebugTextWidget::create(this);
    addWidget(m_debugTextWidget, false);
}


GApp::GApp(const Settings& settings, OSWindow* window, RenderDevice* rd, bool createWindowIfNull) :
    m_lastDebugID(0),
    m_screenCapture(nullptr),
    m_submitToDisplayMode(SubmitToDisplayMode::MAXIMIZE_THROUGHPUT),
    m_settings(settings),
    m_renderPeriod(1),
    m_endProgram(false),
    m_exitCode(0),
    m_debugTextColor(Color3::black()),
    m_debugTextOutlineColor(Color3(0.7f)),
    m_currentEyeIndex(0),
    m_lastFrameOverWait(0),
    debugPane(nullptr),
    renderDevice(nullptr),
    userInput(nullptr),
    m_lastWaitTime(System::time()),
    m_wallClockTargetDuration(1.0f / 60.0f),
    m_lowerFrameRateInBackground(true),
    m_simTimeStep(MATCH_REAL_TIME_TARGET),
    m_simTimeScale(1.0f),
    m_previousSimTimeStep(1.0f / 60.0f),
    m_previousRealTimeStep(1.0f / 60.0f),
    m_realTime(0),
    m_simTime(0) {

    debugAssertM(settings.hdrFramebuffer.colorGuardBandThickness.x >= 0 &&
        settings.hdrFramebuffer.colorGuardBandThickness.y >= 0 &&
        settings.hdrFramebuffer.depthGuardBandThickness.x >= 
        settings.hdrFramebuffer.colorGuardBandThickness.x &&
        settings.hdrFramebuffer.depthGuardBandThickness.y >= 
        settings.hdrFramebuffer.colorGuardBandThickness.y, "The depthGuardBandThickness must include colorGuardBandThickness and be non-negative (see HDRFramebufferSettings documentation)");
    m_gaze = &m_gazeArray[0];
    // Because some subclasses need to run code after GApp is initialized
    // but BEFORE OpenGL is initialized, do not make any OpenGL calls
    // from this constructor unless the window is non-null or 
    // createWindowIfNull is true.

    setCurrent(this);

#   ifdef G3D_DEBUG
        // Let the debugger catch them
        catchCommonExceptions = false;
#   else
        catchCommonExceptions = true;
#   endif

    logLazyPrintf("\nEntering GApp::GApp()\n");
    char b[2048];
    (void)getcwd(b, 2048);
    logLazyPrintf("cwd = %s\n", b);

    if (settings.dataDir == "<AUTO>") {
        dataDir = FilePath::parent(System::currentProgramFilename());
    } else {
        dataDir = settings.dataDir;
    }
    logPrintf("System::setAppDataDir(\"%s\")\n", dataDir.c_str());
    System::setAppDataDir(dataDir);

    if (settings.dataDirs.size() > 0) {
        dataDirs = settings.dataDirs;
    }
    logPrintf("System::setAppDataDirs(");
    for (const String& s : dataDirs) {
        logPrintf("\n\"%s\"", s.c_str());
    }
    logPrintf(")");
    System::setAppDataDirs(dataDirs);

    if (settings.writeLicenseFile && ! FileSystem::exists("g3d-license.txt")) {
        writeLicense();
    }
    
    showDebugText               = true;
    escapeKeyAction             = ACTION_QUIT;
    showRenderingStats          = true;
    manageUserInput             = true;

    // run scm commands before creating opengl context/window
    // determine app scm revision if enabled
    if (m_settings.screenCapture.includeAppRevision || m_settings.screenCapture.addFilesToSourceControl) {
        ScreenCapture::checkAppScmRevision(m_settings.screenCapture.outputDirectory);
    }

    if (createWindowIfNull || notNull(window)) {
        initializeOpenGL(renderDevice, window, createWindowIfNull, settings);
    }
    // Initialize with monocular gaze tracking
    m_gazeTracker = EmulatedGazeTracker::create(this, true);
    logPrintf("Done GApp::GApp()\n\n");
}


CFrame GApp::headFrame() const {
    // Try to find a tracked head
    if (m_scene) {
        const shared_ptr<Entity>& head = m_scene->entity("XR Head");
        if (head) {
            return head->frame();
        }
    }

    // Failed, use the active camera
    if (activeCamera()) {
        return activeCamera()->frame();
    } else {
        return CFrame();
    }
}


const SceneVisualizationSettings& GApp::sceneVisualizationSettings() const {
    if (notNull(developerWindow) && notNull(developerWindow->sceneEditorWindow)) {
        return developerWindow->sceneEditorWindow->sceneVisualizationSettings();
    } else {
        static SceneVisualizationSettings v;
        return v;
    }
}


void GApp::createDeveloperHUD() {
    alwaysAssertM(isNull(developerWindow), "Developer HUD has already been created");

    const shared_ptr<UprightSplineManipulator>& splineManipulator = UprightSplineManipulator::create(m_debugCamera);
    addWidget(splineManipulator);

    const shared_ptr<GFont>&      arialFont = GFont::fromFile(System::findDataFile("arial.fnt"));
    const shared_ptr<GuiTheme>&   theme     = GuiTheme::fromFile(System::findDataFile("osx-10.7.gtm"), arialFont);

    developerWindow = DeveloperWindow::create
       (this,
        m_debugController,
        splineManipulator,
        Pointer<shared_ptr<Manipulator> >(this, &GApp::cameraManipulator, &GApp::setCameraManipulator),
        m_debugCamera,
        scene(),
        theme,
        console,
        Pointer<bool>(debugWindow, &GuiWindow::visible, &GuiWindow::setVisible),
        &showRenderingStats,
        &showDebugText);

    addWidget(developerWindow);
}


shared_ptr<GuiWindow> GApp::show(const shared_ptr<PixelTransferBuffer>& t, const String& windowCaption) {
    bool generateMipMaps = false;
    return show(Texture::fromPixelTransferBuffer("", t, nullptr, Texture::DIM_2D, generateMipMaps), windowCaption);
}


shared_ptr<GuiWindow> GApp::show(const shared_ptr<Image>& t, const String& windowCaption) {
    return show(dynamic_pointer_cast<PixelTransferBuffer>(t->toPixelTransferBuffer()), windowCaption);
}


shared_ptr<GuiWindow> GApp::show(const shared_ptr<Texture>& t, const String& windowCaption) {
    static const Vector2 offset(25, 15);
    static Vector2 lastPos(0, 0);
    static float y0 = 0;

    lastPos += offset;

    String name;
    String dayTime;

    {
        // Use the current time as the name
        time_t t1;
        ::time(&t1);
        tm* t = localtime(&t1);
        static const char* day[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
        int hour = t->tm_hour;
        const char* ap = "am";
        if (hour == 0) {
            hour = 12;
        } else if (hour >= 12) {
            ap = "pm";
            if (hour > 12) {
                hour -= 12;
            }
        }
        dayTime = format("%s %d:%02d:%02d %s", day[t->tm_wday], hour, t->tm_min, t->tm_sec, ap);
    }


    if (! windowCaption.empty()) {
        name = windowCaption + " - ";
    }
    name += dayTime;

    const shared_ptr<GuiWindow>& display = GuiWindow::create(name, shared_ptr<GuiTheme>(), Rect2D::xywh(lastPos,Vector2(0, 0)), GuiTheme::NORMAL_WINDOW_STYLE, GuiWindow::REMOVE_ON_CLOSE);

    GuiTextureBox* box = display->pane()->addTextureBox(this, t);
    box->setSizeFromInterior(t->vector2Bounds().min(Vector2(window()->width() * 0.9f, window()->height() * 0.9f)));
    box->zoomTo1();
    display->pack();

    // Cascade, but don't go off the screen
    if ((display->rect().x1() > window()->width()) || (display->rect().y1() > window()->height())) {
        lastPos = offset;
        lastPos.y += y0;
        y0 += offset.y;

        display->moveTo(lastPos);

        if (display->rect().y1() > window()->height()) {
            y0 = 0;
            lastPos = offset;
            display->moveTo(lastPos);
        }
    }

    addWidget(display);
    return display;
}


void GApp::drawMessage(const String& message) {
    drawTitle(message, "", Any(), Color3::black(), Color4(Color3::white(), 0.8f));
}


void GApp::drawTitle(const String& title, const String& subtitle, const Any& any, const Color3& fontColor, const Color4& backColor) {
    renderDevice->push2D();
    {
        // Background
        renderDevice->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
        Draw::rect2D(renderDevice->viewport(), renderDevice, backColor);
        // Text
        const shared_ptr<GFont> font = debugWindow->theme()->defaultStyle().font;
        const float titleWidth = font->bounds(title, 1).x;
        const float titleSize  = min(30.0f, renderDevice->viewport().width() / titleWidth * 0.80f);
        font->draw2D(renderDevice, title, renderDevice->viewport().center(), titleSize,
                     fontColor, backColor, GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);

        float subtitleSize = 0.0f;
        if (! subtitle.empty()) {
            const float subtitleWidth = font->bounds(subtitle, 1).x;
            subtitleSize = min(22.5f, renderDevice->viewport().width() / subtitleWidth * 0.60f);
            font->draw2D(renderDevice, subtitle, renderDevice->viewport().center() + Vector2(0.0f, font->bounds(title, titleSize).y), subtitleSize,
                         fontColor, backColor, GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
        }

        if (! any.isNil()) {
            any.verifyType(Any::TABLE);
            const float anyTextSize = 20.0f;
            const float baseHeight = renderDevice->viewport().center().y + font->bounds(title, titleSize).y + font->bounds(subtitle, subtitleSize).y;
            const int maxEntriesPerColumn = (int) ((renderDevice->viewport().height() - baseHeight) / font->bounds("l", anyTextSize).y);
            const int cols                = (int) (1 + any.size() / maxEntriesPerColumn);

            Array<String> keys = any.table().getKeys();
            //determine the longest key in order to allign columns well
            Array<float> keyWidth;
            for (int c = 0; c < any.size()/cols; ++c) {
                keyWidth.append(0.0f);
                for (int i = c * maxEntriesPerColumn; i < min((c+1) * maxEntriesPerColumn, any.size()); ++i) {
                    float kwidth = font->bounds(keys[i], anyTextSize).x;
                    keyWidth[c]  = kwidth > keyWidth[c] ? kwidth : keyWidth[c];
                }
            }

            const float horizontalBuffer  = font->bounds("==", anyTextSize).x;
            const float heightIncrement   = font->bounds("==", anyTextSize).y;

            //distance from an edge of a screen to the center of a column, and between centers of columns
            const float centerDist = renderDevice->viewport().width() / (2 * cols);

            for (int c = 0; c < any.size()/cols; ++c) {
                float height = baseHeight;
                for (int i = c * maxEntriesPerColumn; i < min((c+1) * maxEntriesPerColumn, any.size()); ++i) {
                    float columnIndex = 2.0f*c + 1.0f;
                    font->draw2D(renderDevice, keys[i], Vector2(centerDist * (columnIndex) - (horizontalBuffer + keyWidth[c]), height), anyTextSize,
                         fontColor, backColor, GFont::XALIGN_LEFT, GFont::YALIGN_CENTER);
                    font->draw2D(renderDevice, " = ", Vector2(centerDist * (columnIndex), height), anyTextSize,
                         fontColor, backColor, GFont::XALIGN_CENTER, GFont::YALIGN_CENTER);
                    font->draw2D(renderDevice, any[keys[i]].unparse(), Vector2(centerDist * (columnIndex) + horizontalBuffer, height), anyTextSize,
                         fontColor, backColor, GFont::XALIGN_LEFT, GFont::YALIGN_CENTER);
                    height += heightIncrement;
                }
            }
        }
    }
    renderDevice->pop2D();
    renderDevice->swapBuffers();
}


void GApp::setExitCode(int code) {
    m_endProgram = true;
    m_exitCode = code;
}


shared_ptr<GFont> GApp::loadFont(const String& fontName) {
    shared_ptr<GFont> loadedFont;

    logPrintf("Entering GApp::loadFont(\"%s\")\n", fontName.c_str());
    const String& filename = System::findDataFile(fontName);
    logPrintf("Found \"%s\" at \"%s\"\n", fontName.c_str(), filename.c_str());
    if (FileSystem::exists(filename)) {
        loadedFont = GFont::fromFile(filename);
    } else {
        logPrintf(
            "Warning: G3D::GApp could not load font \"%s\".\n"
            "This may be because the G3D::GApp::Settings::dataDir was not\n"
            "properly set in main().\n",
            filename.c_str());
    }
    logPrintf("Done GApp::loadFont(...)\n");
    return loadedFont;
}


GApp::~GApp() {
    if (current() == this) {
        setCurrent(nullptr);
    }

    // Drop pointers to all OpenGL resources before shutting down the RenderDevice
    m_cameraManipulator.reset();

    m_film.reset();
    m_posed3D.clear();
    m_posed2D.clear();
    m_framebuffer.reset();
    m_osWindowHDRFramebuffer.reset();
    m_widgetManager.reset();
    developerWindow.reset();
    debugShapeArray.clear();
    debugLabelArray.clear();

    debugPane = nullptr;
    debugWindow.reset();
    m_debugController.reset();
    m_debugCamera.reset();
    m_activeCamera.reset();

    NetworkDevice::cleanup();

    debugFont.reset();
    delete userInput;
    userInput = nullptr;

    VertexBuffer::cleanupAllVertexBuffers();
    if (!m_hasUserCreatedRenderDevice && m_hasUserCreatedWindow) {
        // Destroy the render device explicitly (rather than waiting for the Window to do so)
        renderDevice->cleanup();
        delete renderDevice;
    }
    renderDevice = nullptr;

    if (! m_hasUserCreatedWindow) {
        delete m_window;
        m_window = nullptr;
    }

    delete m_screenCapture;
    m_screenCapture = nullptr;
}


shared_ptr<Texture> GApp::textureByName(const String& name) const {
    Array<std::weak_ptr<Texture>> all;
    Texture::getAllTextures(all);
    for (int i = 0; i < all.size(); ++i) {
        const shared_ptr<Texture>& t = all[i].lock();
        if (t->name() == name) {
            return t;
        }
    }

    return nullptr;
}


shared_ptr<TextureBrowserWindow> GApp::showInTextureBrowser(const String& textureName, Rect2D rect) {
    const shared_ptr<Texture>& t = textureByName(textureName);
    if (t) {
        return showInTextureBrowser(t, rect);
    } else {
        return nullptr;
    }
}


shared_ptr<TextureBrowserWindow> GApp::showInTextureBrowser(const shared_ptr<Texture>& texture, Rect2D rect) {
    const shared_ptr<TextureBrowserWindow>& browser = TextureBrowserWindow::create(this, texture);
    if (!rect.isEmpty()) {
        browser->moveTo(rect.x0y0());
        browser->setWidth(rect.width());
        browser->setMoveToDefaultLocation(false);
    }
    addWidget(browser);
    browser->setVisible(true);
    return browser;
}


int GApp::run() {
    int ret = 0;
    if (catchCommonExceptions) {
        try {
            onRun();
            ret = m_exitCode;
        } catch (const char* e) {
            alwaysAssertM(false, e);
            ret = -1;
        } catch (const Image::Error& e) {
            alwaysAssertM(false, e.reason + "\n" + e.filename);
            ret = -1;
        } catch (const String& s) {
            alwaysAssertM(false, s);
            ret = -1;
        } catch (const TextInput::WrongTokenType& t) {
            alwaysAssertM(false, t.message);
            ret = -1;
        } catch (const TextInput::WrongSymbol& t) {
            alwaysAssertM(false, t.message);
            ret = -1;
        } catch (const LightweightConduit::PacketSizeException& e) {
            alwaysAssertM(false, e.message);
            ret = -1;
        } catch (const ParseError& e) {
            alwaysAssertM(false, e.formatFileInfo() + e.message);
            ret = -1;
        } catch (const FileNotFound& e) {
            alwaysAssertM(false, e.message);
            ret = -1;
        } catch (const std::exception& e) {
            alwaysAssertM(false, e.what());
            ret = -1;
        }
    } else {
        onRun();
        ret = m_exitCode;
    }

    return ret;
}


void GApp::onRun() {
    if (window()->requiresMainLoop()) {

        // The window push/pop will take care of
        // calling beginRun/oneFrame/endRun for us.
        window()->pushLoopBody(this);

    } else {
        beginRun();

        debugAssertGLOk();
        // Main loop
        do {
            oneFrame();
        } while (! m_endProgram);

        endRun();
    }
}


void GApp::loadScene(const String& sceneName) {
    // Use immediate mode rendering to force a simple message onto the screen
#   ifndef GLEW_EGL
        drawMessage("Loading " + sceneName + "...");
#   endif

    const String oldSceneName = scene()->name();

    // Load the scene
    try {
        m_activeCameraMarker->setTrack(nullptr);

        const Any& any = scene()->load(sceneName);

        // If the debug camera was active and the scene is the same as before, retain the old camera.  Otherwise,
        // switch to the default camera specified by the scene.

        if ((oldSceneName != scene()->name()) || (activeCamera()->name() != "(Debug Camera)")) {

            // Because the CameraControlWindow is hard-coded to the
            // m_debugCamera, we have to copy the camera's values here
            // instead of assigning a pointer to it.
            m_debugCamera->copyParametersFrom(scene()->defaultCamera());
            m_debugCamera->setTrack(nullptr);
            m_debugController->setFrame(m_debugCamera->frame());

            setActiveCamera(scene()->defaultCamera());
        }
        // Re-insert the active camera marker
        scene()->insert(m_activeCameraMarker);

        onAfterLoadScene(any, sceneName);

    } catch (const ParseError& e) {
        const String& msg = e.filename + format(":%d(%d): ", e.line, e.character) + e.message;
        debugPrintf("%s", msg.c_str());
        logPrintf("%s", msg.c_str());
        drawMessage(msg);
        System::sleep(5);
        scene()->clear();
        scene()->lightingEnvironment().ambientOcclusion = m_ambientOcclusion;
    }

    // Trigger one frame of rendering, to force shaders to load and compile
    m_posed3D.fastClear();
    m_posed2D.fastClear();
    if (scene()) {
        onPose(m_posed3D, m_posed2D);
    }

    onGraphics(renderDevice, m_posed3D, m_posed2D);

    // Reset our idea of "now" so that simulation doesn't see a huge lag
    // due to the scene load time.
    m_lastTime = m_now = System::time() - 0.0001f;
}


void GApp::saveScene() {
    // Called when the "save" button is pressed
    if (scene()) {
        Any a = scene()->toAny();
        const String& filename = a.source().filename;
        if (filename != "") {
            a.save(filename);
            debugPrintf("Saved %s\n", filename.c_str());
        } else {
            debugPrintf("Could not save: empty filename");
        }
    }
}


bool GApp::onEvent(const GEvent& event) {
    if (event.type == GEventType::VIDEO_RESIZE) {
        resize(event.resize.w, event.resize.h);
        // Don't consume the resize event--we want subclasses to be able to handle it as well
        return false;
    }

    if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::F5)) {

        String oldCaption = window()->caption();
        window()->setCaption(oldCaption + " (Reloading shaders...)");
        Shader::reloadAll();
        window()->setCaption(oldCaption);
        return true;

    } else if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::F8)) {

        Array<shared_ptr<Texture> > output;
        renderCubeMap(renderDevice, output, m_debugCamera, shared_ptr<Texture>(), 2048);
        drawMessage("Saving Cube Map...");
        const CubeMapConvention::CubeMapInfo& cubeMapInfo = Texture::cubeMapInfo(CubeMapConvention::DIRECTX);
        for (int f = 0; f < 6; ++f) {
            const CubeMapConvention::CubeMapInfo::Face& faceInfo = cubeMapInfo.face[f];
            const shared_ptr<Image>& temp = Image::fromPixelTransferBuffer(output[f]->toPixelTransferBuffer(ImageFormat::RGB32F()));
            temp->rotateCW(toRadians(-90.0f) * faceInfo.rotations);
            if (faceInfo.flipY) { temp->flipVertical();   }
            if (faceInfo.flipX) { temp->flipHorizontal(); }
            temp->save(format("cube-%s.exr", faceInfo.suffix.c_str()));
        }
        return true;

    } else if (event.type == GEventType::FILE_DROP) {

        Array<String> fileArray;
        window()->getDroppedFilenames(fileArray);
        const String lowerFilename = toLower(fileArray[0]);
        if (endsWith(lowerFilename, ".scn.any") || endsWith(fileArray[0], ".Scene.Any")) {

            // Load a scene
            loadScene(fileArray[0]);
            return true;

        } else if (endsWith(fileArray[0], ".ArticulatedModel.Any") ||
            endsWith(fileArray[0], ".MD3Model.Any") ||
            endsWith(fileArray[0], ".MD2Model.Any") ||
            endsWith(fileArray[0], ".Heightfield.Any") ||
            endsWith(lowerFilename, ".gltf") || endsWith(lowerFilename, ".glb")  ||
            endsWith(lowerFilename, ".3ds") || endsWith(lowerFilename, ".ifs")  ||
            endsWith(lowerFilename, ".obj") || endsWith(lowerFilename, ".ply2") ||
            endsWith(lowerFilename, ".off") || endsWith(lowerFilename, ".ply")  ||
            endsWith(lowerFilename, ".bsp") || endsWith(lowerFilename, ".stl")  ||
            endsWith(lowerFilename, ".lwo") || endsWith(lowerFilename, ".stla") ||
            endsWith(lowerFilename, ".dae") || endsWith(lowerFilename, ".fbx")) {

            // Trace a ray from the drop point
            Model::HitInfo hitInfo;
            scene()->intersect(scene()->eyeRay(activeCamera(), Vector2(event.drop.x + 0.5f, event.drop.y + 0.5f), renderDevice->viewport(), settings().hdrFramebuffer.depthGuardBandThickness), ignoreFloat, false, Array<shared_ptr<Entity> >(), hitInfo);

            if (hitInfo.point.isNaN()) {
                // The drop wasn't on a surface, so choose a point in front of the camera at a reasonable distance
                const CFrame& cframe = activeCamera()->frame();
                hitInfo.set(shared_ptr<Model>(), shared_ptr<Entity>(), shared_ptr<Material>(), Vector3::unitY(), cframe.lookVector() * 4 + cframe.translation);
            }

            // Insert a Model
            Any modelAny;

            // If a 3d model file was dropped, generate an ArticulatedModel::Specification
            // using ArticulatedModelDialog
            if (endsWith(lowerFilename, ".3ds") || endsWith(lowerFilename, ".ifs")  ||
                endsWith(lowerFilename, ".obj") || endsWith(lowerFilename, ".ply2") ||
                endsWith(lowerFilename, ".gltf") || endsWith(lowerFilename, ".glb") ||
                endsWith(lowerFilename, ".off") || endsWith(lowerFilename, ".ply")  ||
                endsWith(lowerFilename, ".bsp") || endsWith(lowerFilename, ".stl")  ||
                endsWith(lowerFilename, ".lwo") || endsWith(lowerFilename, ".stla") ||
                endsWith(lowerFilename, ".dae") || endsWith(lowerFilename, ".fbx")) {

                shared_ptr<ArticulatedModelSpecificationEditorDialog> amd = ArticulatedModelSpecificationEditorDialog::create(window(), debugWindow->theme());
                ArticulatedModel::Specification spec = ArticulatedModel::Specification();
                spec.filename = fileArray[0];
                spec.scale = 1.0f;
                amd->getSpecification(spec);
                modelAny = spec.toAny();
            } else {
                // Otherwise just load the dropped file
                modelAny.load(fileArray[0]);
                // Make the filename of the model findable, as the name can no longer be relative to the .Any file
                // this means giving the full path if necessary, or the path after G3D10DATA
                String fileName = modelAny.get("filename", modelAny.get("directory", "")).string();

                fileName = FilePath::concat(FilePath::parent(fileArray[0]), fileName);
                static Array<String> dataPaths;
                if (dataPaths.length() == 0) {
                    System::initializeDirectoryArray(dataPaths);
                }

                for (int i = 0; i < dataPaths.size(); ++i) {
                    if (beginsWith(fileName, dataPaths[i])) {
                        fileName = fileName.substr(dataPaths[i].length());
                    }
                }

                fileName = FilePath::canonicalize(fileName);
                if (modelAny.containsKey("filename")) {
                    modelAny["filename"] = fileName;
                } else if (modelAny.containsKey("directory")) {
                    modelAny["directory"] = fileName;
                }
            }
            int nameModifier = 0;
            Array<String> entityNames;
            scene()->getEntityNames(entityNames);
            // creates a unique name in order to avoid conflicts from multiple models being dragged in
            String name = makeValidIndentifierWithUnderscores(FilePath::base(fileArray[0]));

            if (entityNames.contains(name)) {
                while (entityNames.contains(format("%s%d", name.c_str(), ++nameModifier)));
                name = name.append(format("%d", nameModifier));
            }

            const String& newModelName  = format("%s%d", name.c_str(), nameModifier);
            const String& newEntityName = name;

            scene()->createModel(modelAny, newModelName);

            Any entityAny(Any::TABLE, "VisibleEntity");
            // Insert an Entity for that model
            entityAny["frame"] = CFrame(hitInfo.point);
            entityAny["model"] = newModelName;

            scene()->createEntity("VisibleEntity", newEntityName, entityAny);
            return true;
        }
    } else if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'g') && (activeCamera() == m_debugCamera)) {
        Model::HitInfo info;
        Vector2 mouse;
        uint8 ignore;
        window()->getRelativeMouseState(mouse, ignore);
        const shared_ptr<Entity>& selection =
            scene()->intersect(scene()->eyeRay(activeCamera(), mouse + Vector2(0.5f, 0.5f), renderDevice->viewport(),
                        settings().hdrFramebuffer.depthGuardBandThickness), ignoreFloat, sceneVisualizationSettings().showMarkers, Array<shared_ptr<Entity> >(), info);

        if (notNull(selection)) {
            m_debugCamera->setFrame(CFrame(m_debugCamera->frame().rotation, info.point + (m_debugCamera->frame().rotation * Vector3(0.0f, 0.0f, 1.5f))));
            m_debugController->setFrame(m_debugCamera->frame());
        }
    }

    return false;
}


void GApp::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    if (! scene()) {
        if ((submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) && (! rd->swapBuffersAutomatically())) {
            swapBuffers();
        }
        rd->clear();
        rd->pushState(); {
            rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
            drawDebugShapes();
        } rd->popState();
        return;
    }

    BEGIN_PROFILER_EVENT("GApp::onGraphics3D");
    GBuffer::Specification gbufferSpec = m_gbufferSpecification;
    extendGBufferSpecification(gbufferSpec);
    m_gbuffer->setSpecification(gbufferSpec);

    const Vector2int32 framebufferSize = m_settings.hdrFramebuffer.hdrFramebufferSizeFromDeviceSize(Vector2int32(m_deviceFramebuffer->vector2Bounds()));
    m_framebuffer->resize(framebufferSize);
    m_gbuffer->resize(framebufferSize);
    m_gbuffer->prepare(rd, activeCamera(), 0, -(float)previousSimTimeStep(), m_settings.hdrFramebuffer.depthGuardBandThickness, m_settings.hdrFramebuffer.colorGuardBandThickness);
    
    m_renderer->render(rd, activeCamera(), m_framebuffer, scene()->lightingEnvironment().ambientOcclusionSettings.enabled ? m_depthPeelFramebuffer : nullptr, 
        scene()->lightingEnvironment(), m_gbuffer, allSurfaces, [&]() -> decltype(auto) { return scene()->tritree(); }); // decltype(auto) for correct return type deduction in the lambda.

    // Debug visualizations and post-process effects
    rd->pushState(m_framebuffer); {
        // Call to make the App show the output of debugDraw(...)
        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
        drawDebugShapes();
        const shared_ptr<Entity>& selectedEntity = (notNull(developerWindow) && notNull(developerWindow->sceneEditorWindow)) ? developerWindow->sceneEditorWindow->selectedEntity() : nullptr;
        scene()->visualize(rd, selectedEntity, allSurfaces, sceneVisualizationSettings(), activeCamera());

        onPostProcessHDR3DEffects(rd);
    } rd->popState();

    // We're about to render to the actual back buffer, so swap the buffers now.
    // This call also allows the screenshot and video recording to capture the
    // previous frame just before it is displayed.
    if (submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) {
        swapBuffers();
    }

    // Clear the entire screen (needed even though we'll render over it, since
    // AFR uses clear() to detect that the buffer is not re-used.)
    BEGIN_PROFILER_EVENT("RenderDevice::clear");
    rd->clear();
    END_PROFILER_EVENT();


    // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), 
        settings().hdrFramebuffer.trimBandThickness().x,
        settings().hdrFramebuffer.depthGuardBandThickness.x,
        Texture::opaqueBlackIfNull(notNull(m_gbuffer) ? m_gbuffer->texture(GBuffer::Field::SS_POSITION_CHANGE) : nullptr),
        activeCamera()->jitterMotion());
    END_PROFILER_EVENT();
}


void GApp::onPostProcessHDR3DEffects(RenderDevice* rd) {
    // Post-process special effects
    m_depthOfField->apply(rd, m_framebuffer->texture(0), m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(), m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);

    m_motionBlur->apply(rd, m_framebuffer->texture(0), m_gbuffer->texture(GBuffer::Field::SS_POSITION_CHANGE),
                        m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(),
                        m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);
}


void GApp::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    Surface2D::sortAndRender(rd, posed2D);
}


void GApp::onGraphics(RenderDevice* rd, Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    debugAssertGLOk();

    rd->pushState(); {
        debugAssert(notNull(activeCamera()));
        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
        debugAssertGLOk();
        onGraphics3D(rd, posed3D);
    } rd->popState();
    
    if (notNull(m_screenCapture)) {
        m_screenCapture->onAfterGraphics3D(rd);
    }

    rd->push2D(); {
        onGraphics2D(rd, posed2D);
    } rd->pop2D();

    if (notNull(m_screenCapture)) {
        m_screenCapture->onAfterGraphics2D(rd);
    }
}


void GApp::addWidget(const shared_ptr<Widget>& module, bool setFocus) {
    m_widgetManager->add(module);

    if (setFocus) {
        m_widgetManager->setFocusedWidget(module);
    }
}


void GApp::removeWidget(const shared_ptr<Widget>& module) {
    m_widgetManager->remove(module);
}


void GApp::resize(int w, int h) {

    // ensure a minimum size before guard band
    w = max(8, w);
    h = max(8, h);
    if (notNull(developerWindow)) {
        const Vector2 developerBounds = developerWindow->bounds().wh();
        developerWindow->setRect(Rect2D::xywh(Point2(float(w), float(h)) / developerWindow->pixelScale() - developerBounds, developerBounds));
    }

    // Resize the OS window's buffers only
    const Vector2int32 hdrSize = m_settings.hdrFramebuffer.hdrFramebufferSizeFromDeviceSize(Vector2int32(w, h));

    // Does the m_osWindowHDRFramebuffer need to be reallocated?  Do this even if we
    // aren't using it at the moment, but not if we are minimized.
    shared_ptr<Texture> color0(m_osWindowHDRFramebuffer->texture(0));

    if (notNull(m_film) && ! window()->isIconified() &&
        (isNull(color0) ||
        (m_osWindowHDRFramebuffer->width() != hdrSize.x) ||
        (m_osWindowHDRFramebuffer->height() != hdrSize.y))) {

        m_osWindowHDRFramebuffer->clear();

        const ImageFormat* colorFormat = GLCaps::firstSupportedTexture(m_settings.hdrFramebuffer.preferredColorFormats);
        const ImageFormat* depthFormat = GLCaps::firstSupportedTexture(m_settings.hdrFramebuffer.preferredDepthFormats);
        const bool generateMipMaps = false;
        m_osWindowHDRFramebuffer->set(Framebuffer::COLOR0, Texture::createEmpty("G3D::GApp::m_osWindowHDRFramebuffer/color", hdrSize.x, hdrSize.y, colorFormat, Texture::DIM_2D, generateMipMaps, 1));

        if (notNull(depthFormat)) {
            // Prefer creating a texture if we can
            const Framebuffer::AttachmentPoint p = (depthFormat->stencilBits > 0) ? Framebuffer::DEPTH_AND_STENCIL : Framebuffer::DEPTH;
            alwaysAssertM(GLCaps::supportsTexture(depthFormat), "Depth format not supported");

            // Most applications will reset this to be bound to the GBuffer's depth buffer
            m_osWindowHDRFramebuffer->set(p, Texture::createEmpty("G3D::GApp::m_osWindowHDRFramebuffer/depth", hdrSize.x, hdrSize.y, depthFormat, Texture::DIM_2D, generateMipMaps, 1));

            // This may be resized elsewhere if not rendering to the OS Window, e.g. for VR
            m_depthPeelFramebuffer  = Framebuffer::create(Texture::createEmpty("G3D::GApp::m_depthPeelFramebuffer/depth", hdrSize.x, hdrSize.y, depthFormat, Texture::DIM_2D));
        }
    }
}


void GApp::oneFrame() {
    for (int repeat = 0; repeat < max(1, m_renderPeriod); ++repeat) {
        Profiler::nextFrame();
        m_lastTime = m_now;
        m_now = System::time();
        RealTime timeStep = m_now - m_lastTime;

        // Logic
        m_logicWatch.tick();
        BEGIN_PROFILER_EVENT("GApp::onAI");
        onAI();
        END_PROFILER_EVENT();
        m_logicWatch.tock();

        // User input
        m_userInputWatch.tick();
        if (manageUserInput) {
            processGEventQueue();
        }
        onAfterEvents();
        onUserInput(userInput);
        m_userInputWatch.tock();
        
        if (notNull(m_gazeTracker)) {
            BEGIN_PROFILER_EVENT("GApp::sampleGazeTrackerData");
            sampleGazeTrackerData();
            END_PROFILER_EVENT();
        }

        // Network
        BEGIN_PROFILER_EVENT("GApp::onNetwork");
        m_networkWatch.tick();
        onNetwork();
        m_networkWatch.tock();
        END_PROFILER_EVENT();

        // Simulation
        m_simulationWatch.tick();
        BEGIN_PROFILER_EVENT("Simulation");
        {
            RealTime rdt = timeStep;

            SimTime sdt = m_simTimeStep;
            if (sdt == MATCH_REAL_TIME_TARGET) {
                sdt = m_wallClockTargetDuration;
            } else if (sdt == REAL_TIME) {
                sdt = float(timeStep);
            }
            sdt *= m_simTimeScale;

            SimTime idt = m_wallClockTargetDuration;

            onBeforeSimulation(rdt, sdt, idt);
            onSimulation(rdt, sdt, idt);
            onAfterSimulation(rdt, sdt, idt);

            m_previousSimTimeStep = float(sdt);
            m_previousRealTimeStep = float(rdt);
            setRealTime(realTime() + rdt);
            setSimTime(simTime() + sdt);
        }
        m_simulationWatch.tock();
        END_PROFILER_EVENT();
    }
    
    // Pose
    BEGIN_PROFILER_EVENT("Pose");
    m_poseWatch.tick(); {
        m_posed3D.fastClear();
        m_posed2D.fastClear();
        onPose(m_posed3D, m_posed2D);

        // The debug camera is not in the scene, so we have
        // to explicitly pose it. This actually does nothing, but
        // it allows us to trigger the TAA code.
        m_debugCamera->onPose(m_posed3D);
    } m_poseWatch.tock();
    END_PROFILER_EVENT();

    // Wait
    // Note: we might end up spending all of our time inside of
    // RenderDevice::beginFrame.  Waiting here isn't double waiting,
    // though, because while we're sleeping the CPU the GPU is working
    // to catch up.    
    BEGIN_PROFILER_EVENT("Wait");
    m_waitWatch.tick(); {
        RealTime nowAfterLoop = System::time();

        // Compute accumulated time
        RealTime cumulativeTime = nowAfterLoop - m_lastWaitTime;

        debugAssert(m_wallClockTargetDuration < finf());
        // Perform wait for actual time needed
        RealTime duration = m_wallClockTargetDuration;
        if (! window()->hasFocus() && m_lowerFrameRateInBackground) {
            // Lower frame rate
            duration = 1.0 / BACKGROUND_FRAME_RATE;
        }
        RealTime desiredWaitTime = max(0.0, duration - cumulativeTime);
        onWait(max(0.0, desiredWaitTime - m_lastFrameOverWait) * 0.97);

        // Update wait timers
        m_lastWaitTime = System::time();
        RealTime actualWaitTime = m_lastWaitTime - nowAfterLoop;

        // Learn how much onWait appears to overshoot by and compensate
        double thisOverWait = actualWaitTime - desiredWaitTime;
        if (abs(thisOverWait - m_lastFrameOverWait) / max(abs(m_lastFrameOverWait), abs(thisOverWait)) > 0.4) {
            // Abruptly change our estimate
            m_lastFrameOverWait = thisOverWait;
        } else {
            // Smoothly change our estimate
            m_lastFrameOverWait = lerp(m_lastFrameOverWait, thisOverWait, 0.1);
        }
    }  m_waitWatch.tock();
    END_PROFILER_EVENT();

    debugAssertGLOk();
    if ((submitToDisplayMode() == SubmitToDisplayMode::BALANCE) && (! renderDevice->swapBuffersAutomatically())) {
        swapBuffers();
    }

    BEGIN_PROFILER_EVENT("Graphics");
    renderDevice->beginFrame();
    m_widgetManager->onBeforeGraphics();
    m_graphicsWatch.tick(); {
        debugAssertGLOk();
        renderDevice->pushState(); {
            debugAssertGLOk();
            onGraphics(renderDevice, m_posed3D, m_posed2D);
        } renderDevice->popState();
    }  m_graphicsWatch.tock();
    renderDevice->endFrame();
    if ((submitToDisplayMode() == SubmitToDisplayMode::MINIMIZE_LATENCY) && (!renderDevice->swapBuffersAutomatically())) {
        swapBuffers();
    }
    END_PROFILER_EVENT();

    // Remove all expired debug shapes
    for (int i = 0; i < debugShapeArray.size(); ++i) {
        if (debugShapeArray[i].endTime <= m_now) {
            debugShapeArray.fastRemove(i);
            --i;
        }
    }

    for (int i = 0; i < debugLabelArray.size(); ++i) {
        if (debugLabelArray[i].endTime <= m_now) {
            debugLabelArray.fastRemove(i);
            --i;
        }
    }

    debugText.fastClear();

    m_posed3D.fastClear();
    m_posed2D.fastClear();

    if (m_endProgram && window()->requiresMainLoop()) {
        window()->popLoopBody();
    }
}


void GApp::swapBuffers() {
    BEGIN_PROFILER_EVENT("GApp::swapBuffers");
    renderDevice->swapBuffers();
    END_PROFILER_EVENT();
}


void GApp::drawDebugShapes() {
    BEGIN_PROFILER_EVENT("GApp::drawDebugShapes");

    renderDevice->setObjectToWorldMatrix(CFrame());

    if (debugShapeArray.size() > 0) {

        renderDevice->setPolygonOffset(-1.0f);
        for (int i = 0; i < debugShapeArray.size(); ++i) {
            const DebugShape& s = debugShapeArray[i];
            s.shape->render(renderDevice, s.frame, s.solidColor, s.wireColor);
        }
        renderDevice->setPolygonOffset(0.0f);
    }

    if (debugLabelArray.size() > 0) {
        renderDevice->pushState(); {
            renderDevice->setDepthWrite(false);
            for (int i = 0; i < debugLabelArray.size(); ++i) {
                const DebugLabel& label = debugLabelArray[i];
                if (! label.text.text().empty()) {
                    static const shared_ptr<GFont> defaultFont = GFont::fromFile(System::findDataFile("arial.fnt"));
                    const shared_ptr<GFont>& f = label.text.element(0).font(defaultFont);
                    f->draw3DBillboard(renderDevice, label.text, label.wsPos, label.size, label.text.element(0).color(Color3::black()), Color4::clear(), label.xalign, label.yalign);
                }
            }
        } renderDevice->popState();
    }
    END_PROFILER_EVENT();
}


void GApp::removeAllDebugShapes() {
    debugShapeArray.fastClear();
    debugLabelArray.fastClear();
}


void GApp::removeDebugShape(DebugID id) {
    for (int i = 0; i < debugShapeArray.size(); ++i) {
        if (debugShapeArray[i].id == id) {
            debugShapeArray.fastRemove(i);
            return;
        }
    }
}


void GApp::onWait(RealTime t) {
    System::sleep(max(0.0, t));
}


void GApp::setRealTime(RealTime r) {
    m_realTime = r;
}


void GApp::setSimTime(SimTime s) {
    m_simTime = s;
}


void GApp::onInit() {
    // create screen capture after data directory is set and opengl initialized
    const shared_ptr<GFont>& devFont = GFont::fromFile(System::findDataFile(m_settings.developerToolsFontName));
    const shared_ptr<GuiTheme>& devTheme = GuiTheme::fromFile(System::findDataFile(m_settings.developerToolsThemeName), devFont);
    m_screenCapture = new ScreenCapture(devTheme, this);

    debugAssert(notNull(m_ambientOcclusion));
    setScene(Scene::create(m_ambientOcclusion));

    // Detect scene files in additional data directories.
    if (!dataDirsAddedToScene) {
        Array<String> additionalDataDirs = { dataDir };
        for (const String& s : dataDirs) {
            additionalDataDirs.append(s);
        }
        scene()->appendSceneSearchPaths(additionalDataDirs);
        dataDirsAddedToScene = true;
    }

#   ifndef GLEW_EGL
    if (m_settings.useDeveloperTools) { 
        createDeveloperHUD();
    }
#   endif
}


void GApp::onCleanup() {}


void GApp::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    if (notNull(m_cameraManipulator)) { m_cameraManipulator->setEnabled(activeCamera() == m_debugCamera); }

    m_widgetManager->onSimulation(rdt, sdt, idt);
    
    {
        const CFrame p = m_debugCamera->frame();
        if (m_cameraManipulator) {
            m_debugCamera->setFrame(m_cameraManipulator->frame(), false);
        }

        // Update the camera's TAA jitter.
        m_debugCamera->onSimulation(0, idt);
        m_debugCamera->setPreviousFrame(p);
    }

    if (activeCamera() == m_debugCamera) {
        // The debug camera is not simulated by the scene, so we must explicitly move the 
        // m_activeCameraMarker to match it instead of using a Entity::Track.
        m_activeCameraMarker->setFrame(m_debugCamera->frame());
    }

    if (scene()) { scene()->onSimulation(sdt); }

    // If our renderer is (a subclass of) DefaultRenderer...
    const shared_ptr<DefaultRenderer>& defaultRenderer = dynamic_pointer_cast<DefaultRenderer>(m_renderer);
    if (notNull(defaultRenderer)) {
        // ...update the dynamic diffuse GI
        defaultRenderer->updateDiffuseGI(renderDevice, scene(), m_gbuffer, activeCamera());
    }
}


void GApp::onBeforeSimulation(RealTime& rdt, SimTime& sdt, SimTime& idt) {
    (void)idt;
    (void)rdt;
    (void)sdt;
}


void GApp::onAfterSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    // screenPrintf("%f \n", m_activeCameraMarker->frame().translation.x - m_activeCamera->frame().translation.x);
#ifndef G3D_NO_FMOD
    if (notNull(m_activeListener) && notNull(AudioDevice::instance)) {
        const Vector3& velocity = 
            (isFinite(sdt) && (sdt > 0.0)) ? 
            (m_activeListener->frame().translation - m_activeListener->previousFrame().translation) / sdt :
            Vector3::zero();
        AudioDevice::instance->setListener3DAttributes(m_activeListener->frame(), velocity);
    }
#endif

    (void)idt;
    (void)rdt;
    (void)sdt;
}


void GApp::onPose(Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) {
    m_widgetManager->onPose(surface, surface2D);

    if (scene()) {
        scene()->onPose(surface);
    }
}


void GApp::onNetwork() {
    m_widgetManager->onNetwork();
}


void GApp::onAI() {
    m_widgetManager->onAI();
}


void GApp::beginRun() {

    m_endProgram = false;
    m_exitCode = 0;

    onInit();

    // Move the controller to the camera's location
    if (notNull(m_cameraManipulator) && notNull(m_debugCamera)) {
        m_cameraManipulator->setFrame(m_debugCamera->frame());
    }

    m_now = System::time() - 0.001;
}


void GApp::endRun() {
    onCleanup();

    Log::common()->section("Files Used");

#ifdef G3D_WINDOWS
    // mark required DLL's as used on Windows
    Log::common()->println("cudart64_102.dll");
    Log::common()->println("embree.dll");
    Log::common()->println("optix.6.0.0.dll");
    Log::common()->println("tbb.dll");
    Log::common()->println("tbbmalloc.dll");

#ifndef G3D_NO_FFMPEG
    Log::common()->println("avcodec-58.dll");
    Log::common()->println("avdevice-58.dll");
    Log::common()->println("avfilter-7.dll");
    Log::common()->println("avformat-58.dll");
    Log::common()->println("avutil-56.dll");
    Log::common()->println("postproc-55.dll");
    Log::common()->println("swresample-3.dll");
    Log::common()->println("swscale-5.dll");
#endif

#ifndef G3D_NO_FMOD
    Log::common()->println("fmod.dll");
#endif
#endif // G3D_WINDOWS

    Array<String> fileArray;
    FileSystem::usedFiles().getMembers(fileArray);

    // Canonicalize file names
    for (int i = 0; i < fileArray.size(); ++i) {
        fileArray[i] = FilePath::canonicalize(FileSystem::resolve(fileArray[i]));
    }

    // Alphabetize
    fileArray.sort();

    // Print
    for (int i = 0; i < fileArray.size(); ++i) {
        Log::common()->println(fileArray[i]);
    }
    Log::common()->println("");

    if (window()->requiresMainLoop() && m_endProgram) {
        ::exit(m_exitCode);
    }
}


void GApp::staticConsoleCallback(const String& command, void* me) {
    ((GApp*)me)->onConsoleCommand(command);
}


void GApp::onConsoleCommand(const String& cmd) {
    if (trimWhitespace(cmd) == "exit") {
        setExitCode(0);
        return;
    }
}


void GApp::onUserInput(UserInput* userInput) {
    m_widgetManager->onUserInput(userInput);
}


void GApp::processGEventQueue() {
    userInput->beginEvents();

    // Event handling
    GEvent event;
    while (window()->pollEvent(event)) {
        bool eventConsumed = false;

        // For event debugging
        //if (event.type != GEventType::MOUSE_MOTION) {
        //    printf("%s\n", event.toString().c_str());
        //}

        eventConsumed = WidgetManager::onEvent(event, m_widgetManager);

        if (! eventConsumed) {

            eventConsumed = onEvent(event);

            if (! eventConsumed) {
                switch(event.type) {
                case GEventType::QUIT:
                    setExitCode(0);
                    break;

                case GEventType::KEY_DOWN:

                    if (isNull(console) || ! console->active()) {
                        switch (event.key.keysym.sym) {
                        case GKey::ESCAPE:
                            switch (escapeKeyAction) {
                            case ACTION_QUIT:
                                setExitCode(0);
                                break;

                            case ACTION_SHOW_CONSOLE:
                                console->setActive(true);
                                eventConsumed = true;
                                break;

                            case ACTION_NONE:
                                break;
                            }
                            break;

                        // Add other key handlers here
                        default:;
                        }
                    }
                    break;

                // Add other event handlers here

                default:;
                }
            } // consumed
        } // consumed


        // userInput sees events if they are not consumed, or if they are release events
        if (! eventConsumed || (event.type == GEventType::MOUSE_BUTTON_UP) || (event.type == GEventType::KEY_UP)) {
            // userInput always gets to process events, so that it
            // maintains the current state correctly.
            userInput->processEvent(event);
        }
    }

    userInput->endEvents();
}


void GApp::onAfterEvents() {
    m_widgetManager->onAfterEvents();
}


void GApp::setActiveCamera(const shared_ptr<Camera>& camera) {
    if (camera != m_activeCamera) {

        if (scene()->contains(m_activeCamera) && scene()->contains(camera)) {
            if (m_activeCameraMarker->track()) {
                // Remove the old dependency
                scene()->clearOrder(m_activeCamera->name(), m_activeCameraMarker->name());
            }

            if (camera == m_debugCamera) {
                m_activeCameraMarker->setTrack(nullptr);
            } else if (scene()) {
                // Follow the new camera
                m_activeCameraMarker->setTrack(Entity::EntityTrack::create(&*m_activeCameraMarker, camera->name(), &(*scene())));
            }
        }

        // Teleport the m_activeCameraMarker to the new position
        // so that it has a reasonable velocity
        m_activeCameraMarker->setFrame(camera->frame(), false);
        m_activeCameraMarker->setPreviousFrame(camera->previousFrame());

        m_activeCamera = camera;
    }
}


void GApp::extendGBufferSpecification(GBuffer::Specification& spec) {
    if (notNull(scene())) {
        scene()->lightingEnvironment().ambientOcclusionSettings.extendGBufferSpecification(spec);
        activeCamera()->motionBlurSettings().extendGBufferSpecification(spec);
        activeCamera()->depthOfFieldSettings().extendGBufferSpecification(spec);
        activeCamera()->filmSettings().extendGBufferSpecification(spec);
    }
}


void GApp::renderCubeMap(RenderDevice* rd, Array<shared_ptr<Texture> >& output, const shared_ptr<Camera>& camera, const shared_ptr<Texture>& depthMap, int resolution) {

    Array<shared_ptr<Surface> > surface;
    {
        Array<shared_ptr<Surface2D> > ignore;
        onPose(surface, ignore);
    }
    const int oldFramebufferWidth       = m_osWindowHDRFramebuffer->width();
    const int oldFramebufferHeight      = m_osWindowHDRFramebuffer->height();
    const Vector2int16  oldColorGuard   = m_settings.hdrFramebuffer.colorGuardBandThickness;
    const Vector2int16  oldDepthGuard   = m_settings.hdrFramebuffer.depthGuardBandThickness;
    const shared_ptr<Camera>& oldCamera = activeCamera();

    m_settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(128, 128);
    m_settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(256, 256);
    const int fullWidth = resolution + (2 * m_settings.hdrFramebuffer.depthGuardBandThickness.x);
    m_osWindowHDRFramebuffer->resize(fullWidth, fullWidth);

    shared_ptr<Camera> newCamera = Camera::create("Cubemap Camera");
    newCamera->copyParametersFrom(camera);
    newCamera->setTrack(nullptr);
    newCamera->depthOfFieldSettings().setEnabled(false);
    newCamera->motionBlurSettings().setEnabled(false);
    newCamera->setFieldOfView(2.0f * ::atan(1.0f + 2.0f*(float(m_settings.hdrFramebuffer.depthGuardBandThickness.x) / float(resolution)) ), FOVDirection::HORIZONTAL);

    const ImageFormat* imageFormat = ImageFormat::RGB16F();
    if ((output.size() == 0) || isNull(output[0])) {
        // allocate cube maps
        output.resize(6);
        for (int face = 0; face < 6; ++face) {
            output[face] = Texture::createEmpty(CubeFace(face).toString(), resolution, resolution, imageFormat, Texture::DIM_2D, false);
        }
    }

    // Configure the base camera
    CFrame cframe = newCamera->frame();

    setActiveCamera(newCamera);
    for (int face = 0; face < 6; ++face) {
        Texture::getCubeMapRotation(CubeFace(face), cframe.rotation);
        newCamera->setFrame(cframe);

        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());

        // Render every face twice to let the screen space reflection/refraction texture to stabilize
        onGraphics3D(rd, surface);
        onGraphics3D(rd, surface);

        Texture::copy(m_osWindowHDRFramebuffer->texture(0), output[face], 0, 0, 1, 
            Vector2int16((m_osWindowHDRFramebuffer->texture(0)->vector2Bounds() - output[face]->vector2Bounds()) / 2.0f),
            CubeFace::POS_X, CubeFace::POS_X, nullptr, false);
    }
    setActiveCamera(oldCamera);
    m_osWindowHDRFramebuffer->resize(oldFramebufferWidth, oldFramebufferHeight);
    m_settings.hdrFramebuffer.colorGuardBandThickness = oldColorGuard;
    m_settings.hdrFramebuffer.depthGuardBandThickness = oldDepthGuard;
}


void GApp::sampleGazeTrackerData() {
    m_gazeTracker->getTrackedGaze(m_gazeArray[0], m_gazeArray[1]);
}

} // namespace
