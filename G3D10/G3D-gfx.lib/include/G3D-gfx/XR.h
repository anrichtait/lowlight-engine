/**
  \file G3D-gfx.lib/include/G3D-gfx/XR.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_XR_h

#include "G3D-base/platform.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/Vector2uint32.h"
#include "G3D-base/Projection.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/AABox.h"
#include "G3D-base/Queue.h"
#include "G3D-base/Ray.h"
#include "G3D-gfx/GKey.h"
#include "G3D-app/UserInput.h"


namespace G3D {

class GazeTracker;
class RenderDevice;
class Framebuffer;
class Texture;
class UserInput;
   
/** \brief Low-level eXtended Reality (AR/VR/MR) API base class, analogous to 2D GUI OSWindow for a VR system.

    Subclasses should provide a static create() method
    which instantiates the object but performs no intialization
    (because the graphics context and other parts of the system
    will not be initialized themselves yet at instantiation time).
    
    All coordinate frames are relative to the the TrackedVolume. See G3D::XRWidget
    for a high-level API that manages frames in world space.

    \see OpenVR, MonitorXR, XRWidget    
*/
class XR : public ReferenceCountedObject {
public:

    class TrackedVolume {
    public:
        AABox           boxBounds;

        /** Only the XZ coordinates are used */
        Array<Point3>   polygonBounds;
    };


    class Object : public ReferenceCountedObject {
    protected:
        friend class XR;

        CFrame                  m_frame;
        CFrame                  m_previousFrame;
        String                  m_name;
        int                     m_index;
        int                     m_nativeAPIIndex;

        Object(int index, int nativeAPIIndex, const String& name);

    public:

        /** Index of this object in the underlying OS API (e.g., OVR, OpenVR, SteamVR) */
        int nativeAPIIndex() const {
            return m_nativeAPIIndex;
        }

        static shared_ptr<Object> create(int index, int nativeAPIIndex, const String& name) {
            return createShared<Object>(index, nativeAPIIndex, name);
        }
        
        /** Where the AR/VR system estimates the object _will be_ at the time of the next
            HMD::submit() call, in the room coordinate frame. */
        const CFrame& frame() const {
            return m_frame;
        }

        /** Frame that was predicted at the previous HMD::submit() time. */
        const CFrame& previousFrame() const {
            return m_previousFrame;
        }

        /** Unique identifier. */
        const String& name() const {
            return m_name;
        }
        
        /** Zero-based index in the XR::objectArray */
        const int index() const {
            return m_index;
        }

        // Type queries avoid the overhead of dynamic_cast
        virtual bool isController() const { return false; }
        virtual bool isHMD() const { return false; }
    };
    
    /** Allows XR subclasses to access protected variables inside a tracked object */
    static void setFrame(const shared_ptr<Object>& t, const CFrame& f, const CFrame& p);

    /** Wand with buttons and joysticks or equivalent. 
        The order of button press and release within a frame is not available under this API,
        however, it is almost impossible for a human to press *and* release a button in 11 ms 
        for a 90 Hz renderer so the distinction is likely irrelevant.

        \sa G3D::GameController, G3D::UserInput, G3D::GEvent, G3D::XR::Hand  */
    class Controller : public Object {
    protected:
        Controller(int index, int nativeAPIIndex, const String& name) : Object(index, nativeAPIIndex, name) {}

    public:
        /** Was this button pressed at least once since the previous simulation frame? */
        virtual bool justPressed(GKey b) const = 0;

        /** Was this button released at least once since the previous simulation frame? */
        virtual bool justReleased(GKey b) const = 0;

        /** State (true = pressed, false = released) of the button at the end of the frame. */
        virtual bool currentlyDown(GKey b) const = 0;

        /** Value of the joystick axis \a a */

        /** Returns the counter-clockwise angle in radians that the stick has rotated through since the last
            HMD::submit*/
        virtual float angleDelta(JoystickIndex s) const = 0;

        virtual Vector2 stickPosition(JoystickIndex s) const = 0;

        virtual Vector2 delta(JoystickIndex s) const = 0;

        virtual bool isController() const override { return true; }

        /** \sa hasTouchpad */
        virtual bool hasPhysicalJoystick() const = 0;

        /** \sa hasPhysicalJoystick */
        virtual bool hasTouchpad() const = 0;

        /** Name of an .ArticulatedModel.Any file to use as the 3D representation of this 
            controller. */
        virtual String modelFilename() const = 0;

        /** Is currently in the right hand */
        virtual bool isRight() const = 0;

        /** Is currently in the left hand */
        virtual bool isLeft() const = 0;
    };


    /** Tracked human hand. 
    
        \image html xr-hand-bones.png
    */
    class Hand : public Object {
    protected:

        Array<CFrame>       m_boneArray;

        Hand(int index, int nativeAPIIndex, const String& name) : Object(index, nativeAPIIndex, name) {}

    public:

        /** For each finger, 0 is the bone nearest the wrist.        
         */
        // Do not reorder; these intentionally match the SteamVR HandSkeletonBone indexing scheme
        G3D_DECLARE_ENUM_CLASS(BoneIndex,
	        ROOT,
	        WRIST,

	        THUMB0,
	        THUMB1,
	        THUMB2,
	        THUMB3,

	        INDEX0,
	        INDEX1,
            INDEX2,
            INDEX3,
            INDEX4,

	        MIDDLE0,
            MIDDLE1,
            MIDDLE2,
            MIDDLE3,
            MIDDLE4,

	        RING0,
            RING1,
            RING2,
            RING3,
            RING4,

	        PINKY0,
            PINKY1,
            PINKY2,
            PINKY3,
            PINKY4
        );
        
        /** Coordinate frame for each bone (joint).
            These are relative to the *root bone*,
            not the tracking space, the parent bone,
            or world space.
             
            \sa BoneIndex 
        */
        const Array<CFrame>& boneFrameArray() const {
            return m_boneArray;
        }

        static shared_ptr<Hand> create(int index, int nativeAPIIndex, const String& name) {
            return createShared<Hand>(index, nativeAPIIndex, name);
        }
    };


    /**
    Display attached to the head.

    --------------

    GameController automatically runs in emulation mode, overriding/supplementing any 
    actual controllers found. It defaults to XR::Controller mode.

    In emulation mode using an GameController:

    Head mode:
    - when in this mode, we override the true head
    - rstick = rotate yaw and pitch
    - lstick = translate XY
    - dpad up/dn = translate Z
    - TBD buttons = slow/fast
    - dpad lt/rt = switch to XR::Controller mode

    XR::Controller mode:
    - sticks and buttons correspond to Rift/Vive controllers
    - dpad lt/rt = switch to head mode if allowGameControllerToDriveHead == true
    */
    class HMD : public Object {
    protected:
        shared_ptr<Controller>  m_leftController;
        shared_ptr<Controller>  m_rightController;
        shared_ptr<Hand>        m_leftHand;
        shared_ptr<Hand>        m_rightHand;
        shared_ptr<GazeTracker> m_gazeTracker;

        HMD(int index, int nativeAPIIndex, const String& name) : Object(index, nativeAPIIndex, name) {}

    public:

        bool allowGameControllerToDriveHead = true;

        /** Top of the user's head in the real world when standing */
        virtual float standingHeadHeight() const = 0;

        /** Does the user prefer to use the right (true) or left (false) hand? */
        virtual bool rightHanded() const = 0;

        /** Is the user's right (true) or left (false) eye dominant? */
        virtual bool rightEyeDominant() const = 0;

        virtual void submitFrame
        (RenderDevice*                    rd, 
         const shared_ptr<Framebuffer>*   hmdDeviceFramebuffer) = 0;

        virtual int numViews() const = 0;

        /** \param viewToHead Must have numViews() elements.
            \param viewProjection Must have numViews() elements.
            
            These may change *every frame* in some displays, such as those with dynamic foveal insets
            or that adjust the eyebox based on gaze tracking.
          */
        virtual void getViewCameraMatrices
        (float                            nearPlaneZ, 
         float                            farPlaneZ,
         CFrame*                          viewToHead,
         Projection*                      viewProjection) const = 0;

        /** The left and right may be the same Texture. They may also be Texture::black() if there is no passthrough video. */
        virtual void getPassThroughVideo
        (shared_ptr<Texture>&             left,
         shared_ptr<Texture>&             right) const = 0;

        /** If there is no tracked left controller, one is created and fixed relative to the head.
            It can be driven using an GameController. */
        const shared_ptr<Controller>& leftController() const {
            return m_leftController;
        }

        /** If there is no hand tracking, it is inferred from the controller */
        const shared_ptr<Hand>& leftHand() const {
            return m_leftHand;
        }

        /** If there is no hand tracking, it is inferred from the controller */
        const shared_ptr<Hand>& rightHand() const {
            return m_rightHand;
        }

        /** If there is no tracked right controller, one is created and fixed relative to the head.
            It can be driven using an GameController. */
        const shared_ptr<Controller>& rightController() const {
            return m_rightController;
        }

        /** If the HMD has no true gaze tracker, it will create an emulation one that always looks forward. */
        const shared_ptr<GazeTracker>& gazeTracker() const {
            return m_gazeTracker;
        }

        /** In Hz */
        virtual float displayFrequency() const = 0;

        /** Device-requested resolution for each view, before warping */
        virtual void getResolution(Vector2uint32* resPerView) const = 0;
        
        virtual bool isHMD() const { return true; }
    };

    
    /** We don't provide motion or button events because those are polled directly from the
        trackedObjectArray(). 
        
        \sa GEvent */
    class Event {
    public:
        enum Type { OBJECT_CREATED, OBJECT_DESTROYED };
        Type                            type;
        shared_ptr<Object>              object;

    protected:
        Event(Type t, const shared_ptr<Object>& obj) : type(t), object(obj) {}

    public:
        static shared_ptr<Event> create(Type t, const shared_ptr<Object>& obj) {
            return createShared<Event>(t, obj);
        }
    };

protected:

    Queue<shared_ptr<XR::Event>>        m_eventQueue;
    TrackedVolume                       m_trackedVolume;
    Array<shared_ptr<Object>>           m_objectArray;
    Array<shared_ptr<HMD>>              m_hmdArray;

    XR() {}

public:
    
    /** Returns nullptr when out of events */
    virtual shared_ptr<Event> nextEvent() {
        return (m_eventQueue.size() > 0) ? m_eventQueue.popFront() : nullptr;
    }

    /** All tracked objects, including HMDs */
    const Array<shared_ptr<Object>>& objectArray() const {
        return m_objectArray;
    }
    
    const Array<shared_ptr<HMD>>& hmdArray() const {
        return m_hmdArray;
    }
    
    const TrackedVolume& trackedVolume() const {
        return m_trackedVolume;
    }

    /** Reserved for future use */
    class Settings {
    
    public:
        class UserInput* userInput = nullptr;
    
    };
    
    /** Call once per frame */
    virtual void updateTrackingData() = 0;

    virtual void preGraphicsInit(const Settings& settings) {}
    virtual void postGraphicsInit(const Settings& settings) {}
    virtual void cleanup() {}

    /** Which subclass of XR is this? */
    virtual const String& className() const = 0;
};

} // namespace G3D

