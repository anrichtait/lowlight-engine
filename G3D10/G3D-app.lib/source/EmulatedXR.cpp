/**
  \file G3D-app.lib/source/EmulatedXR.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/EmulatedXR.h"
#include "G3D-base/Log.h"
#include "G3D-app/Widget.h"

#ifdef G3D_WINDOWS

namespace G3D {

void EmulatedXR::updateTrackingData() {
    const CFrame& frame = m_manipulator ? m_manipulator->frame() : Point3(0.0f, 1.7f, 0.0f);

    if (isNull(m_hmd)) {
        m_hmd = EmulatedHMD::create(m_objectArray.size(), 0, "XR Head", this);
        setFrame(m_hmd, frame, frame);
        m_objectArray.append(m_hmd);
        m_hmdArray.append(m_hmd);
        m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, m_hmd));

        const shared_ptr<EmulatedXRController>& controller = EmulatedXRController::create(this, int(m_objectArray.size()), 1, "XR Right Hand", true);
		const CFrame handFrame;
        setFrame(controller, handFrame, handFrame);
        m_objectArray.append(controller);
        m_controllerArray.append(controller);
        m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, controller));
        //First controller we add is the right hand.
        m_rightHand = controller;
        // TODO add another controller

        // TODO: Drive head with mouse, body with arrows

    } else {
        setFrame(m_hmd, frame, m_hmd->frame());
    }

    //Update controller button events
    for (const shared_ptr<EmulatedXRController>& controller : m_controllerArray) {
        alwaysAssertM(notNull(controller), "Null controller");
        
    }
}


/** Call every frame to obtain the head and eye transformations */
void EmulatedXR::EmulatedHMD::getViewCameraMatrices
   (float                            nearPlaneZ,
    float                            farPlaneZ,
    CFrame*                          viewToHead,
    Projection*                      viewProjection) const {

    assert((nearPlaneZ < 0.0f) && (farPlaneZ < nearPlaneZ));


	const Array<EmulatedXR::Specification::View>& viewArray = m_xr->m_specification.viewArray;
	for (int i = 0; i < numViews(); ++i) {
		const EmulatedXR::Specification::View& view = viewArray[i];
		viewToHead[i] = view.viewFrame;
		Projection projection;
		projection.setFarPlaneZ(farPlaneZ);
		projection.setNearPlaneZ(nearPlaneZ);
		projection.setFieldOfView(view.fieldOfView, view.fovDirection);
		viewProjection[i] = projection;
	}
}


void EmulatedXR::EmulatedHMD::submitFrame
(RenderDevice*                    rd,
 const shared_ptr<Framebuffer >*  hmdDeviceFramebuffer) {

    // Do nothing, as we are only simulating an HMD.
}


float EmulatedXR::EmulatedHMD::displayFrequency() const {
	return m_xr->m_specification.displayFrequency;
}

int EmulatedXR::EmulatedHMD::numViews() const {
	return m_xr->m_specification.viewArray.length();
}

void EmulatedXR::EmulatedHMD::getResolution(Vector2uint32* resPerView) const {
	for (int i = 0; i < m_xr->m_specification.viewArray.length(); ++i) {
		resPerView[i] = m_xr->m_specification.viewArray[i].resolution;
	}
}

} // Namespace
#endif