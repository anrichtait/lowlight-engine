/**
  \file G3D-gfx.lib/source/MonitorXR.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-gfx/MonitorXR.h"
#include "G3D-base/Log.h"

#ifdef G3D_WINDOWS

namespace G3D {

void MonitorXR::MonitorHMD::init(OSWindow::Settings settings) {
    settings.asynchronous = false;
    settings.sharedContext = true;
    settings.depthBits = 0;
    m_window = nullptr;

    const OSWindow* w = OSWindow::current();
    m_window = GLFWWindow::create(settings);
    w->makeCurrent();
}


void MonitorXR::updateTrackingData() {
    const CFrame& frame = Matrix4::translation(0.0f, 1.78f, 0.0f).approxCoordinateFrame();
    if (isNull(m_hmd)) {
        m_hmd = MonitorHMD::create(m_objectArray.size(), 0, "XR Head", this, m_settings);
        setFrame(m_hmd, frame, frame);
        m_objectArray.append(m_hmd);
        m_hmdArray.append(m_hmd);
        m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, m_hmd));
    } else {
        setFrame(m_hmd, frame, m_hmd->frame());
    }
}


MonitorXR::MonitorHMD::MonitorHMD(int index, int nativeAPIIndex, const String& name, MonitorXR* xr) :  HMD(index, nativeAPIIndex, name), m_xr(xr) {}


/** Call every frame to obtain the head and eye transformations */
void MonitorXR::MonitorHMD::getViewCameraMatrices
   (float                            nearPlaneZ,
    float                            farPlaneZ,
    CFrame*                          viewToHead,
    Projection*                      viewProjection) const {

    assert((nearPlaneZ < 0.0f) && (farPlaneZ < nearPlaneZ));

    viewToHead[0] = CFrame(Matrix3::identity());
    viewToHead[1] = CFrame(Matrix3::identity());

    // Vive's near plane code is off by a factor of 2 according to G3D
    //const vr::HmdMatrix44_t& ltProj = m_xr->m_system->GetProjectionMatrix(vr::Eye_Left, -nearPlaneZ * 2.0f, -farPlaneZ);
    //const vr::HmdMatrix44_t& rtProj = m_xr->m_system->GetProjectionMatrix(vr::Eye_Right, -nearPlaneZ * 2.0f, -farPlaneZ);

    const Matrix4& ltProj = Matrix4::perspectiveProjection(-0.1f, 0.1f, -0.1f, 0.1f, -nearPlaneZ, -farPlaneZ, 1.0f); //something reasonable
    const Matrix4& rtProj = Matrix4::perspectiveProjection(-0.1f, 0.1f, -0.1f, 0.1f, -nearPlaneZ, -farPlaneZ, 1.0f);

    Matrix4 L, R;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            L[r][c] = ltProj[r][c];
            R[r][c] = rtProj[r][c];
        }
    }

    Vector2uint32 res[2];
    getResolution(res);
    viewProjection[0] = Projection(L, Vector2(float(res[0].x), float(res[0].y)));
    viewProjection[1] = Projection(R, Vector2(float(res[1].x), float(res[1].y)));
}


void MonitorXR::MonitorHMD::submitFrame
   (RenderDevice*                    rd,
    const shared_ptr<Framebuffer >*  _hmdDeviceFramebuffer) {

    // Clear the event queue
    for (GEvent ignore; m_window->pollEvent(ignore);) { }

    OSWindow::pushGraphicsContext(m_window); {
        // We *should* be able to directly blit from _hmdDeviceFramebuffer...but that blit fails
        // for unknown reasons, so we bind to a second set of framebuffers (which is inefficient,
        // at least if the textures are changing every frame. We've optimized for the textures
        // being reused every frame, at least)
        for (int i = 0; i < 2; ++i) {
            if (isNull(m_bogus[i])) {
                m_bogus[i] = Framebuffer::create(format("m_bogus[%d]", i));
            }

            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_bogus[i]->openGLID());
            const GLint texID = _hmdDeviceFramebuffer[i]->get(Framebuffer::AttachmentPoint::COLOR0)->texture()->openGLID();
            if (m_texIDs[i] != texID) { // attach new texture object if necessary
                glFramebufferTexture(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID, 0);
                m_texIDs[i] = texID;
            }
        }

        const int w = m_window->width() / 2;
        for (int i = 0; i < 2; ++i) {
            // TODO: use _hmdDeviceFramebuffer[i] instead of m_bogus[i] here
            glBlitNamedFramebuffer(m_bogus[i]->openGLID(), GL_ZERO, 0, 0,
                _hmdDeviceFramebuffer[i]->width(),
                _hmdDeviceFramebuffer[i]->height(),
                w * i, 0, w + w * i, m_window->height(), GL_COLOR_BUFFER_BIT, GL_LINEAR);  
        }

        m_window->swapGLBuffers();
    } OSWindow::popGraphicsContext();
}
}
#endif