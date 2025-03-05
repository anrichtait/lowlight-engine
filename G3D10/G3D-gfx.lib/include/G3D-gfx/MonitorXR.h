/**
  \file G3D-gfx.lib/include/G3D-gfx/MonitorXR.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define G3D_gfx_OpenVR_h

#include "G3D-base/platform.h"
#ifdef G3D_WINDOWS

#include "G3D-base/CoordinateFrame.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Framebuffer.h"
#include "GLFWWindow.h"
#include "G3D-gfx/XR.h"

namespace G3D {

class MonitorXR : public XR {
protected:

    class MonitorHMD : public HMD {

    protected:
        MonitorXR*                      m_xr        = nullptr;
        OSWindow*                       m_window    = nullptr;
        shared_ptr<Framebuffer>         m_bogus[2];

        mutable GLint                   m_texIDs[2] = { 0, 0 };

        virtual void init(OSWindow::Settings settings);

        MonitorHMD(int index, int nativeAPIIndex, const String& name, MonitorXR* xr);

    public:

        static shared_ptr<MonitorHMD> create(int index, int nativeAPIIndex, const String& name, MonitorXR* xr, OSWindow::Settings& settings) {
            shared_ptr<MonitorHMD> h = createShared<MonitorHMD>(index, nativeAPIIndex, name, xr);
            h->init(settings);
            return h;
        }

        virtual float standingHeadHeight() const override {
            return 1.78f;
        }

        virtual bool rightHanded() const override {
            return true;
        }

        virtual bool rightEyeDominant() const override {
            return true;
        }

        virtual void submitFrame
        (RenderDevice*                    rd,
         const shared_ptr<Framebuffer >*  hmdDeviceFramebuffer) override;

        virtual void getViewCameraMatrices
        (float                            nearPlaneZ,
         float                            farPlaneZ,
         CFrame*                          viewToHead,
         Projection*                      viewProjection) const override;

        virtual int numViews() const override {
            return 2;
        }

        /** The left and right may be the same Texture. They may also be Texture::black() if there is no passthrough video. */
        virtual void getPassThroughVideo
        (shared_ptr<Texture>&            left,
         shared_ptr<Texture>&            right) const {}

        /** In Hz */
        virtual float displayFrequency() const {
            return 60.0f;
        }

        virtual void getResolution(Vector2uint32* res) const {
            if (m_window) {
                res[0] = res[1] = Vector2uint32(m_window->width() / 2, m_window->height());
            } else {
                res[0] = res[1] = Vector2uint32(640, 400);
            }
        }

    };

    shared_ptr<MonitorHMD>           m_hmd;

    /** Used for creating the HMD on the first frame. Can't be done when
        MonitorXR is initialized because that happens before OpenGL is initialized. */
    OSWindow::Settings               m_settings;

    MonitorXR(const OSWindow::Settings& settings) : m_settings(settings) {}

public:

    static shared_ptr<MonitorXR> create(const OSWindow::Settings& settings = OSWindow::Settings()) {
        return createShared<MonitorXR>(settings);
    }

    virtual void updateTrackingData() override;

    virtual void preGraphicsInit(const Settings& settings) override {}
    virtual void postGraphicsInit(const Settings& settings) override {}
    virtual void cleanup() override {}
    virtual const String& className() const override { static const String n = "MonitorXR"; return n; }
};
}
#endif