/**
  \file G3D-gfx.lib/include/G3D-gfx/OpenVR.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_OpenVR_h

#include "G3D-base/platform.h"
#ifdef G3D_WINDOWS

#ifdef _MSC_VER
#   pragma comment(lib, "openvr_api")
#endif

#include "G3D-base/CoordinateFrame.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-gfx/XR.h"

// OpenVR forward declarations
namespace vr {
class IVRSystem;
typedef uint64_t TrackedCameraHandle_t;
}

namespace G3D {

/** 
 Implementation of the G3D::XR interface using the OpenVR API by Valve.
 Supports Oculus Rift, Vive, DK2, and OSVR devices.

 This class is G3D-gfx because it directly makes use of graphics-API
 specific calls such as `vr::IVRTrackedCamera::GetVideoStreamTextureGL`
 and `vr::IVRCompositor::Submit`.

 \see G3D::VRApp, G3D::XRWidget
*/
class OpenVR : public XR {
protected:

    class OpenVRController : public Controller {
    protected:
        friend OpenVR;

        // Same as structures and code in GameController

        class Button {
        public:
            bool                    currentValue;

            /** Changed since the previous onAfterEvents */
            bool                    changed;

            Button() : currentValue(false), changed(false) {}
        };

        class Stick {
        public:
            Vector2                 currentValue;
            Vector2                 previousValue;
        };

        static const int NUM_BUTTONS = GKey::CONTROLLER_RIGHT_TRIGGER - GKey::CONTROLLER_A + 1;
        static const int NUM_STICKS  = 6;

        Button                      m_buttonArray[NUM_BUTTONS];
        Stick                       m_stickArray[NUM_STICKS];

        // End same as GameController
    
        /** False for the left... */
        bool                        m_isRight = true;
        bool                        m_hasTouchpad = false;

        String                      m_modelFilename;

        OpenVRController(int index, int openVRIndex, const String& name, bool isRight, bool hasTouchpad, const String& modelFilename) :
            Controller(index, openVRIndex, name), m_isRight(isRight), m_hasTouchpad(hasTouchpad), m_modelFilename(modelFilename) {}

        virtual void update(vr::IVRSystem*);

        /** Performs range checking */
        const Button& button(GKey k) const;

        /** Performs range checking */
        const Stick& stick(JoystickIndex s) const;

    public:

        bool isRight() const override {
            return m_isRight;
        }

        bool isLeft() const override {
            return ! m_isRight;
        }

        static shared_ptr<OpenVRController> create(int index, int openVRIndex, const String& name, bool isRight, bool hasTouchpad, const String& modelFilename) {
            return createShared<OpenVRController>(index, openVRIndex, name, isRight, hasTouchpad, modelFilename);
        }

        virtual bool justPressed(GKey k) const override {
            return button(k).currentValue && button(k).changed;
        }

        virtual bool justReleased(GKey k) const override {
            return ! button(k).currentValue && button(k).changed;
        }

        virtual bool currentlyDown(GKey k) const override{
            return button(k).currentValue;
        }

        virtual float angleDelta(JoystickIndex s) const override;
        virtual Vector2 stickPosition(JoystickIndex s) const override {
            return stick(s).currentValue;
        }

        virtual Vector2 delta(JoystickIndex s) const override {
            const Stick& st = stick(s);
            return st.currentValue - st.previousValue;
        }

        virtual bool hasPhysicalJoystick() const { return ! m_hasTouchpad; }

        /** \sa hasPhysicalJoystick */
        virtual bool hasTouchpad() const { return m_hasTouchpad; }

        String modelFilename() const override;
    };

    class OpenVRHMD : public HMD {
    protected:
        friend class OpenVR;

        OpenVR*     m_xr;
        mutable vr::TrackedCameraHandle_t m_TrackedCameraHandle = 0;
        OpenVRHMD(int index, int openVRIndex, const String& name, OpenVR* xr) : HMD(index, openVRIndex, name), m_xr(xr) {}

        virtual ~OpenVRHMD();

        void setLeftController(const shared_ptr<OpenVRController>& controller) {
            m_leftController = controller;
        }

        void setRightController(const shared_ptr<OpenVRController>& controller) {
            m_rightController = controller;
        }
        
    public:

        static shared_ptr<OpenVRHMD> create(int index, int openVRIndex, const String& name, OpenVR* xr) {
            return createShared<OpenVRHMD>(index, openVRIndex, name, xr);
        }

        virtual float standingHeadHeight() const override;

        virtual bool rightHanded() const override;
          
        virtual bool rightEyeDominant() const override;

        virtual void submitFrame
        (RenderDevice*                    rd, 
         const shared_ptr<Framebuffer >*  hmdDeviceFramebuffer) override;

        virtual void getViewCameraMatrices
        (float                            nearPlaneZ, 
         float                            farPlaneZ,
         CFrame*                          viewToHead,
         Projection*                      viewProjection) const override;

        /** The left and right may be the same Texture. They may also be Texture::black() if there is no passthrough video. */
        virtual void getPassThroughVideo
        (shared_ptr<Texture>&            left,
         shared_ptr<Texture>&            right) const override;
        
        /** In Hz */
        virtual float displayFrequency() const override;

        virtual void getResolution(Vector2uint32* res) const override;
        
        virtual int numViews() const { return 2; }
    };


    float                           m_displayFrequency = 0.0f;
    vr::IVRSystem*                  m_system = nullptr;

    Array<shared_ptr<OpenVRController>> m_controllerArray;

    shared_ptr<OpenVRHMD>           m_hmd;
    shared_ptr<OpenVRController>    m_leftController;
    shared_ptr<OpenVRController>    m_rightController;
    Vector2uint32                   m_resolution;
    bool                            m_hasTouchpad;

    OpenVR() : m_displayFrequency(fnan()) {}

    void setLeftController(const shared_ptr<OpenVRController>& controller);
    void setRightController(const shared_ptr<OpenVRController>& controller);

public:

    static shared_ptr<OpenVR> create() {
        return createShared<OpenVR>();
    }

    virtual const String& className() const override { static const String n = "OpenVR"; return n; }

    virtual void updateTrackingData() override;

    virtual void preGraphicsInit(const Settings& settings) override;
    virtual void postGraphicsInit(const Settings& settings) override;
    virtual void cleanup() override;

    /** Returns true if there is a HMD available on this machine */
    static bool available();
};



} // namespace G3D

#endif

