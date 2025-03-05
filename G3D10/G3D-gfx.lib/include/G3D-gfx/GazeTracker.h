/**
  \file G3D-gfx.lib/include/G3D-gfx/GazeTracker.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_GazerTracker_h

#include "G3D-base/platform.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/Ray.h"


namespace G3D {

  
/** API for low-level gaze tracking. 
   \sa XRWidget, EmulatedGazeTracker, EmulatedXR, OpenVR, XRWidget::gazeRay, GApp::gazeTracker */
class GazeTracker : public ReferenceCountedObject {
public:

    class Gaze {
    public:
        /** For getTrackedGaze(), this is always valid and stable, even during blink. 

           ray.origin is the center of the pupil.

           ray.direction is the visual axis of the eye, which is the center of the fovea
           and attention for a normal eye.

           For a HMD, this is in the "XR Head" frame. For a desktop gaze tracker, it
           is relative to the active camera. \sa GApp::headFrame
        */
        Ray         headSpaceRay;

        /** Dilation of the pupil. Some trackers never update this. Default is 4mm. */
        float       pupilSize = 0.004f;

        /** Is this eye blinking (pupil not visible and \a ray is simulated) */
        bool        inBlink = false;

        /** Is the user in a saccade? (Jump between positions) */
        bool        inSaccade = false;
    };

    /** The application should call with \a c = true to begin calibration, and then
        poll headSpaceCalibrationPoint() every frame to draw the target. Calibration
        will end on its own. To abort calibration early and return to the previous
        calibrated value, invoke `setCalibrationMode(false)`. */
    virtual void setCalibrationMode(bool c) {
        (void)c;
    }

    /** When in calibration mode, the application should draw the calibration target
        at this location in head space. When this returns Point3::nan(), calibration
        has ended. */
    virtual Point3 headSpaceCalibrationPoint() const {
        return Point3::nan();
    }

    /** Raw data with minimal latency. 
    
        Because this will likely be called at the frame rate of the display,
        we recommend that subclasses model the rate and phase of calls and
        only process gaze information on a separate thread right before the
        anticipated time of the next call. This avoids the typical behavior of
        gaze trackers, which is to run continuously at high rates and throw out
        most frames without spending resources processing them unless those 
        intermediate frames are needed to reduce error.

        \sa getTrackedGaze
      */
    virtual void getInstantaneousGaze(Gaze& left, Gaze& right) const = 0;

    /** All values are in world space. You can convert them to screen space
        by the usual method: transform to the camera's reference frame and 
        intersect these rays with the image plane.

        This may be a filtered version of getInstantaneousGaze().

        The default implementation directly returns getInstantaneousGaze(),
        but will likely contain automatic filtering in a future release.
        */
    virtual void getTrackedGaze(Gaze& left, Gaze& right) const {
        getInstantaneousGaze(left, right); 
    }

    /** Which subclass of GazeTracker is this? */
    virtual const String& className() const = 0;
};

} // namespace G3D
