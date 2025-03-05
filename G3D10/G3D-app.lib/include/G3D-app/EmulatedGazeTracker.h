/**
  \file G3D-app.lib/include/G3D-app/EmulatedGazeTracker.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_EmulatedGazeTracker_h

#include "G3D-base/platform.h"
#include "G3D-gfx/GazeTracker.h"

namespace G3D {
     
class GApp;
class GameController;

/** Simulates the presence of a gaze tracker by assuming that the user is always looking
    straight forward, and making the eyes converge on the point hit by a ray cast.
*/
class EmulatedGazeTracker : public GazeTracker {
protected:

    GApp*                           m_app;
    const bool                      m_monocular;
    shared_ptr<GameController>      m_gameController;

    // Calibration data
    mutable RealTime                m_currentCalibrationPointStartTime = 0;

    /** if -1, not calibrating */
    mutable int                     m_calibrationIndex = -1;

    EmulatedGazeTracker(GApp* app, bool monocular, const shared_ptr<GameController>& gameController) : 
        m_app(app), m_monocular(monocular), m_gameController(gameController) {}

public:

    /** The app is used for GApp::scene() ray casts and GApp::headFrame()

        \param monocular If set, no ray casts are used but both eyes act as if they are in the center of the head
        \param gameController If set, use the right stick to control the eye direction relative to the head and the right shoulder button to blink.
    */
    static shared_ptr<EmulatedGazeTracker> create(GApp* app, bool monocular = false, const shared_ptr<GameController>& gameController = nullptr);
    virtual void getInstantaneousGaze(Gaze& left, Gaze& right) const override;
    virtual const String& className() const override;

    virtual void setCalibrationMode(bool c) override;

    virtual Point3 headSpaceCalibrationPoint() const override;
};

} // namespace G3D