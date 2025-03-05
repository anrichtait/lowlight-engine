/**
  \file G3D-app.lib/source/EmulatedGazeTracker.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-app/EmulatedGazeTracker.h"
#include "G3D-app/GApp.h"
#include "G3D-app/Scene.h"
#include "G3D-app/XRWidget.h"
#include "G3D-app/GameController.h"

namespace G3D {

/** Head to left eye (computed from Oculus Rift) */
static const CFrame desktopLeftEyeFrame = CFrame::fromXYZYPRDegrees(-0.031829f, 0, 0, 0, -0, 0);
static const CFrame desktopRightEyeFrame = CFrame::fromXYZYPRDegrees(0.031829f, 0, 0, 0, -0, 0);


static const int NUM_CALIBRATION_POINTS = square(4);
static const RealTime CALIBRATION_DWELL_TIME = 2; // seconds 


void EmulatedGazeTracker::setCalibrationMode(bool c) {
    if (c) {
        if ((m_calibrationIndex == -1) || (m_calibrationIndex >= NUM_CALIBRATION_POINTS)) {
            m_calibrationIndex = 0;
            m_currentCalibrationPointStartTime = System::time();
        }
    } else {
        m_calibrationIndex = -1; 
    }
}


Point3 EmulatedGazeTracker::headSpaceCalibrationPoint() const {
    if (m_calibrationIndex > -1) {
        const RealTime now = System::time();
        const RealTime elapsed = now - m_currentCalibrationPointStartTime;
        if (elapsed >= CALIBRATION_DWELL_TIME) {
            m_currentCalibrationPointStartTime = now;
            ++m_calibrationIndex;
        }
    }

    if ((m_calibrationIndex == -1) || (m_calibrationIndex >= NUM_CALIBRATION_POINTS)) {
        // Done
        m_calibrationIndex = -1;
        return Point3::nan();
    } else {
        const int side = (int)ceil(sqrt(float(NUM_CALIBRATION_POINTS)));
        const int permutation = 0x1B5 % side;
        const int permutedIndex = m_calibrationIndex ^ permutation;
        const float x = 2.0f * float(permutedIndex / side) / float(side) - 1.0f;
        const float y = 2.0f * float(permutedIndex % side) / float(side) - 1.0f;
        return Point3(x, y, -2.0f);
    }
}


shared_ptr<EmulatedGazeTracker> EmulatedGazeTracker::create(GApp* app, bool monocular, const shared_ptr<GameController>& gameController) {
    return createShared<EmulatedGazeTracker>(app, monocular, gameController);
}


void EmulatedGazeTracker::getInstantaneousGaze(Gaze& left, Gaze& right) const {
    const CFrame& headFrame = m_app->headFrame();
    const shared_ptr<Scene>& scene = m_app->scene();

    if (isNull(scene)) {
        //alwaysAssertM(false, "TODO: implement");
        return;
    }

    CFrame eyeFrame[2] = {desktopLeftEyeFrame, desktopRightEyeFrame};

    // 1km is effectively infinity (50m is effectively infinity!)
    float focusDistance = 1000.0f;

    Vector3 direction(0,0,-1.0f);
    bool blink = false;

    if (m_gameController) {
        const Point2 stick = m_gameController->position(JoystickIndex::LEFT);
        direction = Vector3(stick.x, -stick.y, -1.0).direction();
        blink = m_gameController->currentlyDown(GKey::CONTROLLER_RIGHT_BUMPER);
    }

    const Ray& headRay = Ray(Point3::zero(), direction, 0.25f, focusDistance);

    left.inBlink = blink;
    left.inSaccade = false;
    right.inBlink = blink;
    right.inSaccade = false;

    if (m_monocular) {
        left.headSpaceRay = headRay;
        right.headSpaceRay = headRay;
    } else {
        const Ray& cyclopsRay = headFrame.toWorldSpace(headRay);

        const shared_ptr<XRWidget::TrackedEntity>& head = m_app->scene()->typedEntity<XRWidget::TrackedEntity>("XR Head");
        if (notNull(head)) {
            const shared_ptr<XR::HMD>& hmd = dynamic_pointer_cast<XR::HMD>(head->object());
            Projection ignore[2];
            hmd->getViewCameraMatrices(-0.1f, -100.0f, eyeFrame, ignore);
        }

        // Prevent the ray cast from hitting geometry attached to the head itself
        static const Array<String> excludeRootNameArray("XR Head");
        Array<String> excludeNameArray;
        Array<shared_ptr<Entity>> excludeArray;
        if (notNull(head)) {
            scene->getDescendants(excludeRootNameArray, excludeNameArray);
            for (const String& e : excludeNameArray) {
                excludeArray.append(scene->entity(e));
            }
        }

        if (notNull(scene->intersect(cyclopsRay, focusDistance, false, excludeArray))) {
            // Assume the user is never focusing closer than 20 cm 
            focusDistance = max(focusDistance, 0.20f);

            const Point3 hsFocusPoint(headRay.direction() * focusDistance);
            left.headSpaceRay = Ray(eyeFrame[0].translation, (hsFocusPoint - eyeFrame[0].translation).direction());
            right.headSpaceRay = Ray(eyeFrame[1].translation, (hsFocusPoint - eyeFrame[1].translation).direction());
        }
    }
}


const String& EmulatedGazeTracker::className() const {
    static const String n = "EmulatedGazeTracker";
    return n;
}

}
