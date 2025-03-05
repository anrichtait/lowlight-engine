/**
  \file G3D-app.lib/include/G3D-app/EmulatedXR.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define G3D_app_EmulatedXR_h

#include "G3D-base/platform.h"
#ifdef G3D_WINDOWS

#include "G3D-base/CoordinateFrame.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-gfx/XR.h"

namespace G3D {

class EmulatedXR : public XR {
public:

	class Specification {
	public:
		class View {
		public:
            /** View to head transformation */
            CFrame					viewFrame;
            Vector2uint32			resolution;
			FOVDirection			fovDirection;
			float					fieldOfView;

			View() : resolution(1024, 1024), fovDirection(FOVDirection::VERTICAL), fieldOfView(pif() / 2.0f) {}
			View(const CFrame& frame, const int w, const int h, FOVDirection dir, float angle) : viewFrame(frame), resolution(w, h), fovDirection(dir), fieldOfView(angle) {}
		};

		Array<View>					viewArray;
		float						displayFrequency;
		
		Specification() : displayFrequency(60.0f) {
			viewArray.append(View(), View());
		}
	};

protected:

    class EmulatedXRController : public Controller {
    protected:
        friend EmulatedXR;

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
        static const int NUM_STICKS = 6;

        Button                      m_buttonArray[NUM_BUTTONS];
        Stick                       m_stickArray[NUM_STICKS];

        // End same as GameController

        /** False for the left... */
        bool                        m_isRight = true;
        bool                        m_hasTouchpad = false;

		EmulatedXR*					m_xr;
	
		EmulatedXRController(EmulatedXR* xr, int index, int nativeAPIIndex, const String& name, bool isRight) :
            Controller(index, nativeAPIIndex, name), m_isRight(isRight), m_xr(xr) {}

        /** Performs range checking */
        const Button& button(GKey k) const { 
            // alwaysAssertM(false, "TODO: implement");
            static Button b;
            return b;
        }

        /** Performs range checking */
        const Stick& stick(JoystickIndex s) const {
            // alwaysAssertM(false, "TODO: implement");
            static Stick a;
            return a;
        }

    public:

        int openVRIndex = -1;

        bool isRight() const {
            return m_isRight;
        }

        bool isLeft() const {
            return !m_isRight;
        }

        static shared_ptr<EmulatedXRController> create(EmulatedXR* xr, int index, int nativeAPIIndex, const String& name, bool isRight) {
            return createShared<EmulatedXRController>(xr, index, nativeAPIIndex, name, isRight);
        }

        virtual bool justPressed(GKey k) const override {
            return button(k).currentValue && button(k).changed;
        }

        virtual bool justReleased(GKey k) const override {
            return !button(k).currentValue && button(k).changed;
        }

        virtual bool currentlyDown(GKey k) const override {
            return button(k).currentValue;
        }

        virtual float angleDelta(JoystickIndex s) const override {
            alwaysAssertM(false, "TODO: implement");
            return -1.0f;
        }

        virtual Vector2 stickPosition(JoystickIndex s) const override {
            return stick(s).currentValue;
        }

        virtual Vector2 delta(JoystickIndex s) const override {
            const Stick& st = stick(s);
            return st.currentValue - st.previousValue;
        }

        virtual bool hasPhysicalJoystick() const { return false; }

        /** \sa hasPhysicalJoystick */
        virtual bool hasTouchpad() const { return true; }

        String modelFilename() const {
            return System::findDataFile(m_isRight ? "model/vr/rift_cv1_right_controller.ArticulatedModel.Any" : "model/vr/rift_cv1_left_controller.ArticulatedModel.Any");
        }
    };

    class EmulatedHMD : public HMD {
    protected:

        EmulatedXR*                     m_xr;

        //To enforce logical const at the expense of bitwise.
        mutable GLint                   m_texIDs[2] = { 0, 0 };
        
        EmulatedHMD(int index, int nativeAPIIndex, const String& name, EmulatedXR* xr) : HMD(index, nativeAPIIndex, name), m_xr(xr) {}

    public:

        static shared_ptr<EmulatedHMD> create(int index, int nativeAPIIndex, const String& name, EmulatedXR* xr) {
            return createShared<EmulatedHMD>(index, nativeAPIIndex, name, xr);
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

        /** The left and right may be the same Texture. They may also be Texture::black() if there is no passthrough video. */
        virtual void getPassThroughVideo
           (shared_ptr<Texture>&            left,
            shared_ptr<Texture>&            right) const {}

        /** In Hz */
		virtual float displayFrequency() const;

		virtual int numViews() const override;

        /** Ideal resolution for one eye before warping */
		virtual void getResolution(Vector2uint32* resPerView) const;

    };

    Array<shared_ptr<EmulatedXRController>> m_controllerArray;

    shared_ptr<EmulatedHMD>           m_hmd;
    shared_ptr<EmulatedXRController>  m_rightHand;
    shared_ptr<EmulatedXRController>  m_leftHand;

    UserInput*                        m_userInput;

	Specification				      m_specification;
    shared_ptr<class Manipulator>     m_manipulator;

    EmulatedXR(const Specification& specification) : m_specification(specification) {}

public:

    static shared_ptr<EmulatedXR> create(const Specification& specification = Specification()) {
        return createShared<EmulatedXR>(specification);
    }

    /** Allows direct mutation of a View */
    Specification::View& view(int i) {
        return m_specification.viewArray[i];
    }

    void setHMDManipulator(const shared_ptr<class Manipulator>& manipulator) {
        m_manipulator = manipulator;
    }

    virtual void updateTrackingData() override;

    virtual void preGraphicsInit(const Settings& settings) override {
        m_userInput = settings.userInput;
    }
    virtual void postGraphicsInit(const Settings& settings) override {}
    virtual void cleanup() override {}
    virtual const String& className() const override { static const String n = "EmulatedXR"; return n; }
};
}
#endif