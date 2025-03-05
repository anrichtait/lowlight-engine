/**
  \file G3D-gfx.lib/include/G3D-gfx/EGLWindow.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_EGLWindow_h

#include "G3D-base/platform.h"
#include "G3D-gfx/OSWindow.h"

namespace G3D {

class EGLWindow : public OSWindow {
private:

    /** Master EGL context */
    static EGLContext s_share;
    EGLDisplay m_eglDpy;
    EGLContext m_eglCtx;
    EGLSurface m_eglSurf;
    String m_versionString;
    EGLWindow(const OSWindow::Settings& settings);

public:

    /** GUI scaling for high-DPI monitors. */
    static float defaultGuiPixelScale(); 

    virtual void getSettings(OSWindow::Settings& settings) const override;
    
    virtual int width() const override;
    virtual int height() const override;
    
    virtual Rect2D clientRect() const override;

    virtual void setClientRect(const Rect2D &dims) override;

    virtual Rect2D fullRect() const override {
        alwaysAssertM(false, "unimplemented");
        return Rect2D();
    }

    virtual void setFullRect(const Rect2D &dims) override {
        alwaysAssertM(false, "unimplemented");
    }

    virtual void getDroppedFilenames(Array<String>& files) override {
        alwaysAssertM(false, "unimplemented");
    }
    
    virtual void setClientPosition(int x, int y) override;

    virtual void setFullPosition(int x, int y) override;
    
    virtual bool hasFocus() const override;
    
    String getAPIVersion() const override;

    String getAPIName() const override;

    virtual String className() const override { return "EGLWindow"; }

    virtual void setGammaRamp(const Array<uint16>& gammaRamp) override;
    
    virtual void setCaption(const String& title) override;

    virtual String caption() override;

    virtual int numJoysticks() const override;
    
    virtual String joystickName(unsigned int stickNum) const override;

    virtual void setRelativeMousePosition(double x, double y) override;
    virtual void setRelativeMousePosition(const Vector2& p) override;
    virtual void getRelativeMouseState(Vector2 &position, uint8 &mouseButtons) const override;

    virtual void getRelativeMouseState(int &x, int &y, uint8 &mouseButtons) const override;

    virtual void getRelativeMouseState(double &x, double &y, uint8 &mouseButtons) const override;

    virtual void getJoystickState(unsigned int stickNum, Array<float> &axis, Array<bool> &buttons) const override;
    
    virtual void setInputCapture(bool c) override;

    virtual void setMouseVisible(bool b) override;

    virtual bool requiresMainLoop() const override {
        return false;
    }

    virtual void swapGLBuffers() override;
    
    /** \copydoc OSWindow::primaryDisplaySize */
    static Vector2 primaryDisplaySize();

    /** \copydoc OSWindow::primaryDisplayRefreshRate */
    static float primaryDisplayRefreshRate(int width, int height);

    /** \copydoc OSWindow::virtualDisplaySize */
    static Vector2 virtualDisplaySize();

    /** \copydoc OSWindow::primaryDisplayWindowSize */
    static Vector2int32 primaryDisplayWindowSize();

    /** \copydoc OSWindow::numDisplays */
    static int numDisplays();

    static EGLWindow* create(const OSWindow::Settings& settings = OSWindow::Settings());
    
    virtual void setAsCurrentGraphicsContext() const override;

    virtual ~EGLWindow();

    virtual String _clipboardText() const override;

    virtual void _setClipboardText(const String& text) const override;

}; // EGLWindow

} // namespace G3D
