/**
  \file G3D-gfx.lib/source/EGLWindow.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-gfx/GLCaps.h"

#include "G3D-base/Vector2.h"
#include "G3D-base/Rect2D.h"
#include "G3D-gfx/glcalls.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "G3D-gfx/EGLWindow.h"

namespace G3D {

EGLContext G3D::EGLWindow::s_share = EGL_NO_CONTEXT;

EGLWindow::EGLWindow(const OSWindow::Settings& settings) {
    debugPrintf("Initializing EGL Window\n");
    EGLint s_configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    m_settings = settings;

    static const int MAX_DEVICES = 4;
    EGLDeviceEXT eglDevs[MAX_DEVICES];
    EGLint numDevices;

    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
        (PFNEGLQUERYDEVICESEXTPROC)
        eglGetProcAddress("eglQueryDevicesEXT");

    eglQueryDevicesEXT(MAX_DEVICES, eglDevs, &numDevices);

    debugPrintf("EGLWindow: Detected %d devices\n", numDevices);

    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)
        eglGetProcAddress("eglGetPlatformDisplayEXT");

    m_eglDpy = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, 
                                        eglDevs[0], 0);

    EGLint major, minor;
    if (! eglInitialize(m_eglDpy, &major, &minor)) {
        alwaysAssertM(false, "Failed to initialize EGL\n");
    }

    m_versionString = eglQueryString(m_eglDpy, EGL_VERSION);

    // Set config
    EGLint numConfigs;
    EGLConfig eglCfg;
    if (! eglChooseConfig(m_eglDpy, s_configAttribs, &eglCfg, 1, &numConfigs)) {
        alwaysAssertM(false, "Failed to set config\n");
    }

    int pbufferWidth = m_settings.width;
    int pbufferHeight = m_settings.height;
    EGLint pbufferAttribs[] = {
        EGL_WIDTH, pbufferWidth,
        EGL_HEIGHT, pbufferHeight,
        EGL_NONE,
    };

    // Create surface
    m_eglSurf = eglCreatePbufferSurface(m_eglDpy, eglCfg, pbufferAttribs);
    if (! eglBindAPI(EGL_OPENGL_API)) {
        alwaysAssertM(false, "Failed to bind to OpenGL\n");
    }

    // Create context
    m_eglCtx = eglCreateContext(m_eglDpy, eglCfg, s_share, NULL);

    if (s_share == EGL_NO_CONTEXT) {
        s_share = m_eglCtx;
    }

    if (m_eglCtx == EGL_NO_CONTEXT) {
        eglTerminate(m_eglDpy);
        if (s_share == m_eglCtx) {
            s_share = EGL_NO_CONTEXT;
        }
        alwaysAssertM(false, "Failed create context\n");
    }

    if (s_share == m_eglCtx) {
        if (! eglMakeCurrent(m_eglDpy, m_eglSurf, m_eglSurf, m_eglCtx)) {
            alwaysAssertM(false, "Failed to set current context\n");
        }
        GLCaps::init();
    }

    debugAssertGLOk();
}


EGLWindow::~EGLWindow() {
    if (s_share == m_eglCtx) {
        // I'm the master window
        eglTerminate(m_eglDpy);
        s_share = nullptr;
    }
}

float EGLWindow::defaultGuiPixelScale() {
    return 1.0f;
}


void EGLWindow::getSettings(OSWindow::Settings& settings) const {
    settings = m_settings;
}

    
int EGLWindow::width() const {
    return m_settings.width;
}


int EGLWindow::height() const {
    return m_settings.height;
}

    
Rect2D EGLWindow::clientRect() const {
    return Rect2D::xywh((float)0, (float)0, (float)width(), (float)height());
}

void EGLWindow::setClientRect(const Rect2D &dims){
    // EGL cannot resize window
    return;
}
    

void EGLWindow::setClientPosition(int x, int y) {
    // Keep window at x=0, y=0
    return;
}

void EGLWindow::setFullPosition(int x, int y) {
    // Keep window at x=0, y=0
    return;
}
    

bool EGLWindow::hasFocus() const {
    return true;
}
    

String EGLWindow::getAPIVersion() const {
    return m_versionString;
}


String EGLWindow::getAPIName() const{
    return "EGL";
}


void EGLWindow::setGammaRamp(const Array<uint16>& gammaRamp) {
    // Gamma ramp not supported
    return;
}
   

void EGLWindow::setCaption(const String& title) {
    // Caption not supported
    return;
}


String EGLWindow::caption(){
    return "";
}

int EGLWindow::numJoysticks() const {
    // Joystick not supported
    return 0;
}
  

String EGLWindow::joystickName(unsigned int stickNum) const {
    // Joystick not supported
    return "";
}
    
  

void EGLWindow::setRelativeMousePosition(double x, double y) {
    // Mouse not supported
    return;
}


void EGLWindow::setRelativeMousePosition(const Vector2 &p){
    // Mouse not supported
    return;
}


void EGLWindow::getRelativeMouseState(Vector2 &position, uint8 &mouseButtons) const {
    // Mouse not supported
    return;
}


void EGLWindow::getRelativeMouseState(int &x, int &y, uint8 &mouseButtons) const {
    // Mouse not supported
    return;
}


void EGLWindow::getRelativeMouseState(double &x, double &y, uint8 &mouseButtons) const {
    // Mouse not supported
    return;
}


void EGLWindow::getJoystickState(unsigned int stickNum, Array<float> &axis, Array<bool> &buttons) const {
    // Joystick not supported
    return;
}


void EGLWindow::setInputCapture(bool c) {
    return;
}


void EGLWindow::setMouseVisible(bool b){
    return;
}

EGLWindow* EGLWindow::create(const OSWindow::Settings& settings){
    EGLWindow* window = new EGLWindow(settings);
    return window;
}

void EGLWindow::setAsCurrentGraphicsContext() const {
    if (! eglMakeCurrent(m_eglDpy, m_eglSurf, m_eglSurf, m_eglCtx)) {
        alwaysAssertM(false, "Failed to set current context\n");
    }
}

void EGLWindow::swapGLBuffers() {
    eglSwapBuffers(m_eglDpy, m_eglSurf);
    return;
}

float EGLWindow::primaryDisplayRefreshRate(int width, int height) {
    return 144.0f;
}

Vector2 EGLWindow::primaryDisplaySize() {
    return Vector2(4096, 2160);
}

Vector2 EGLWindow::virtualDisplaySize() {
    return Vector2(4096, 2160);
}

Vector2int32 EGLWindow::primaryDisplayWindowSize() {
    return Vector2int32(4096, 2160);
}


int EGLWindow::numDisplays() {
    return 1;
}


String EGLWindow::_clipboardText() const {
    // Clipboard not supported
    return "";
}

void EGLWindow::_setClipboardText(const String& text) const {
    // Clipboard not supported
    return;
}

} // namespace G3D 

