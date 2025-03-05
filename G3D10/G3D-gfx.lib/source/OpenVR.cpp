/**
  \file G3D-gfx.lib/source/OpenVR.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-gfx/OpenVR.h"
#include "G3D-base/Log.h"
#include "G3D-app/GApp.h" // for screenprintf

#ifdef G3D_WINDOWS
#include <openvr/openvr.h>

namespace G3D {

// To avoid requiring the TrackedDevicePose_t and header in the OpenVR.h file
static vr::TrackedDevicePose_t m_trackedDevicePose[vr::k_unMaxTrackedDeviceCount];

/** Maps EVRButtonId to GKey. This only works for 8 buttons
    on Vive. 
    
    
    Rift mapping
    -----------------------------
    ### Buttons (Digital)
    Xbox                Device   Button ID
    A                 =  R          7
    B                 =  R          1
    X                 =  L          7
    Y                 =  L          1
    Ltrigger          =  L         33
    Rtrigger          =  R         33
    Rstick click      =  R         32
    Lstick click      =  L         32
    Lbumper (grip)    =  L     2 & 34
    Rbumper (grip)    =  R     2 & 34

    ### Sticks (Analog)
    Left Stick        =  L          0
    Left Grip         =  L          2 (x only)
    Left Trigger      =  L          1 (x only)
    Right Stick       =  R          0 
    Right Grip        =  R          2 (x only)
    Right Trigger     =  R          1 (x only)


    Vive mapping
    -----------------------------
    ### Buttons
    Xbox                Device   Button ID
    A                 =  -          -
    B                 =  -          -
    X                 =  -          -
    Y                 =  -          -
    Start             =  R          1
    Back              =  L          1
    Ltrigger          =  L         33
    Rtrigger          =  R         33
    Rstick click      =  R         32
    Lstick click      =  L         32
    Lbumper (grip)    =  L          2
    Rbumper (grip)    =  R          2

    ### Sticks (Analog)
    Left Stick        =  L          0
    Left Trigger      =  L          1 (x only)
    Right Stick       =  R          0
    Right Trigger     =  R          1 (x only)
    */


const OpenVR::OpenVRController::Button& OpenVR::OpenVRController::button(GKey k) const {
    alwaysAssertM(k >= GKey::CONTROLLER_A && k <= GKey::CONTROLLER_RIGHT_TRIGGER, "Not a controller button");
    return m_buttonArray[k - GKey::CONTROLLER_A];
}


const OpenVR::OpenVRController::Stick& OpenVR::OpenVRController::stick(JoystickIndex s) const {
    alwaysAssertM(s >= JoystickIndex::LEFT && s <= JoystickIndex::RIGHT_SHOULDER, 
        "Not an analog stick");

    return m_stickArray[s];
}


void OpenVR::OpenVRController::update(vr::IVRSystem* system) {
    // Get new state
    vr::VRControllerState_t state;
    const bool success = system->GetControllerState(m_nativeAPIIndex, &state, sizeof(state));
    // Sometimes a valid openVRIndex still fails here once the OVR driver gets confused
    if (! success) { return; }

    alwaysAssertM(success, format("Invalid device index (%d) passed to OpenVR", m_nativeAPIIndex));

    // Sticks

    // Latch old state
    for (int s = 0; s < NUM_STICKS; ++s) {
        m_stickArray[s].previousValue = m_stickArray[s].currentValue;
    }

    // Update new state. See the large comment above for the mapping from OpenVR axes 
    // to semantic Joystick/GameController axes
    m_stickArray[m_isRight ? JoystickIndex::RIGHT          : JoystickIndex::LEFT         ].currentValue = reinterpret_cast<Vector2&>(state.rAxis[0]);
    m_stickArray[m_isRight ? JoystickIndex::RIGHT_TRIGGER  : JoystickIndex::LEFT_TRIGGER ].currentValue = reinterpret_cast<Vector2&>(state.rAxis[1]);
    m_stickArray[m_isRight ? JoystickIndex::RIGHT_SHOULDER : JoystickIndex::LEFT_SHOULDER].currentValue = reinterpret_cast<Vector2&>(state.rAxis[2]);

    static const GKey openVRBitToGKeyRight[] = 
    {
        GKey::UNKNOWN,                  //  0
        GKey::CONTROLLER_B,             //  1
        GKey::CONTROLLER_RIGHT_BUMPER,  //  2
        GKey::UNKNOWN,                  //  3
        GKey::UNKNOWN,                  //  4
        GKey::UNKNOWN,                  //  5
        GKey::UNKNOWN,                  //  6
        GKey::CONTROLLER_A,             //  7
        GKey::UNKNOWN,                  //  8
        GKey::UNKNOWN,                  //  9
        GKey::UNKNOWN,                  // 10
        GKey::UNKNOWN,                  // 11
        GKey::UNKNOWN,                  // 12
        GKey::UNKNOWN,                  // 13
        GKey::UNKNOWN,                  // 14
        GKey::UNKNOWN,                  // 15
        GKey::UNKNOWN,                  // 16
        GKey::UNKNOWN,                  // 17
        GKey::UNKNOWN,                  // 18
        GKey::UNKNOWN,                  // 19
        GKey::UNKNOWN,                  // 20
        GKey::UNKNOWN,                  // 21
        GKey::UNKNOWN,                  // 22
        GKey::UNKNOWN,                  // 23
        GKey::UNKNOWN,                  // 24
        GKey::UNKNOWN,                  // 25
        GKey::UNKNOWN,                  // 26
        GKey::UNKNOWN,                  // 27
        GKey::UNKNOWN,                  // 28
        GKey::UNKNOWN,                  // 29
        GKey::UNKNOWN,                  // 30
        GKey::UNKNOWN,                  // 31
        GKey::CONTROLLER_RIGHT_CLICK,   // 32
        GKey::CONTROLLER_RIGHT_TRIGGER  // 33
    };
        
    static const GKey openVRBitToGKeyLeft[] = 
    {
        GKey::UNKNOWN,                  //  0
        GKey::CONTROLLER_Y,             //  1
        GKey::CONTROLLER_LEFT_BUMPER,   //  2
        GKey::UNKNOWN,                  //  3
        GKey::UNKNOWN,                  //  4
        GKey::UNKNOWN,                  //  5
        GKey::UNKNOWN,                  //  6
        GKey::CONTROLLER_X,             //  7
        GKey::UNKNOWN,                  //  8
        GKey::UNKNOWN,                  //  9
        GKey::UNKNOWN,                  // 10
        GKey::UNKNOWN,                  // 11
        GKey::UNKNOWN,                  // 12
        GKey::UNKNOWN,                  // 13
        GKey::UNKNOWN,                  // 14
        GKey::UNKNOWN,                  // 15
        GKey::UNKNOWN,                  // 16
        GKey::UNKNOWN,                  // 17
        GKey::UNKNOWN,                  // 18
        GKey::UNKNOWN,                  // 19
        GKey::UNKNOWN,                  // 20
        GKey::UNKNOWN,                  // 21
        GKey::UNKNOWN,                  // 22
        GKey::UNKNOWN,                  // 23
        GKey::UNKNOWN,                  // 24
        GKey::UNKNOWN,                  // 25
        GKey::UNKNOWN,                  // 26
        GKey::UNKNOWN,                  // 27
        GKey::UNKNOWN,                  // 28
        GKey::UNKNOWN,                  // 29
        GKey::UNKNOWN,                  // 30
        GKey::UNKNOWN,                  // 31
        GKey::CONTROLLER_LEFT_CLICK,    // 32
        GKey::CONTROLLER_LEFT_TRIGGER   // 33
    };


    // Buttons
    // Iterate over the relevant *bits* of the state
    for (int i = 0; i < 34; ++i) {
        const GKey b = (m_isRight ? openVRBitToGKeyRight : openVRBitToGKeyLeft)[i];
        if (b != GKey::UNKNOWN) {
            const bool newValue = ((state.ulButtonPressed >> i) & 1) != 0;
            Button& button = m_buttonArray[b - GKey::CONTROLLER_A];
            button.changed = (newValue != button.currentValue);
            button.currentValue = newValue;
        }
    }

    // If we want to later support touched, use:
    // bit mask: state.ulButtonTouched

    if (false) {
        // Code to print the buttons
        String s = m_isRight ? "RT: " : "LT: ", t = "    ";
        for (int i = 0; i < 64; ++i) {
            s += (((state.ulButtonPressed >> i) & 1) != 0) ? " 1 " : " _ ";
            t += format(" %02d", i);
        }
        screenPrintf(s.c_str());
        if (m_isRight) {
            screenPrintf(t.c_str());
        }
    }

    if (false) {
        // Code to print the buttons
        String s = m_isRight ? "RT: " : "LT: ", t = "    ";
        for (int i = 0; i < vr::k_unControllerStateAxisCount; ++i) {
            const Vector2& axis = reinterpret_cast<Vector2&>(state.rAxis[i]);

            s += format("(%5.3f, %5.3f) ", axis.x, axis.y);
            t += format("      %02d       ", i);
        }
        screenPrintf(s.c_str());
        if (m_isRight) {
            screenPrintf(t.c_str());
        }
    }
}


static String getHMDString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr) {
    uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, nullptr, 0, peError);
    if (unRequiredBufferLen == 0) {
        return "";
    }
    
    char* pchBuffer = new char[unRequiredBufferLen];
    unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
    const String& sResult = pchBuffer;
    delete[] pchBuffer;
    
    return sResult;
}


static float getHMDFloat(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = nullptr) {
    return pHmd->GetFloatTrackedDeviceProperty(unDevice, prop, peError);
}    


static CFrame toCFrame(const vr::VRBoneTransform_t& b) {
    return CFrame(Quat(b.orientation.x, b.orientation.y, b.orientation.z, b.orientation.w),
        Point3(b.position.v[0], b.position.v[1], b.position.v[2]));
}


static CFrame toCFrame(const vr::HmdMatrix34_t& M) {
    return CFrame(Matrix3(M.m[0][0], M.m[0][1], M.m[0][2],
                          M.m[1][0], M.m[1][1], M.m[1][2],
                          M.m[2][0], M.m[2][1], M.m[2][2]),
                   Point3(M.m[0][3], M.m[1][3], M.m[2][3]));
}
    
/////////////////////////////////////////////////////////////////


float OpenVR::OpenVRController::angleDelta(JoystickIndex s) const {
    const static float threshold = 0.2f;
    const Stick& st = stick(s);

    if ((st.previousValue.length() < threshold) || (st.currentValue.length() < threshold)) {
        // The stick was too close to the center to measure angles
        return 0.0f;
    } else {
        const float oldAngle = atan2(st.previousValue.y, st.previousValue.x);
        const float newAngle = atan2(st.currentValue.y,  st.currentValue.x);
        float delta = newAngle - oldAngle;

        // Make sure we go the short way

        if (delta > pif()) {
            delta -= 2.0f * pif();
        } else if (delta < -pif()) {
            delta += 2.0f * pif();
        }

        return delta;
    }
}

////////////////////////////////////////////////////////

float OpenVR::OpenVRHMD::standingHeadHeight() const {
    // TODO: Find out how to query this from OpenVR
    return 1.98f;
}


bool OpenVR::OpenVRHMD::rightHanded() const {
    // TODO: Find out how to query this from OpenVR
    return true;
}


bool OpenVR::OpenVRHMD::rightEyeDominant() const {
    return true;
}


void OpenVR::OpenVRHMD::submitFrame
   (RenderDevice*                    rd, 
    const shared_ptr<Framebuffer>*   hmdDeviceFramebuffer) {

    BEGIN_PROFILER_EVENT("OpenVR::OpenVRHMD::submitFrame");
    const vr::EColorSpace colorSpace = vr::ColorSpace_Linear;

    const GLuint id[2] = {
        hmdDeviceFramebuffer[0]->texture(0)->openGLID(),
        hmdDeviceFramebuffer[1]->texture(0)->openGLID()
    };

    debugAssertGLOk();
    for (int eye = 0; eye < 2; ++eye) {
        const vr::Texture_t tex = { reinterpret_cast<void*>(intptr_t(id[eye])), vr::TextureType_OpenGL, colorSpace };
        vr::VRCompositor()->Submit(vr::EVREye(eye), &tex);
    }
#   ifdef G3D_DEBUG
        // Inside the OpenVR driver a GL error is triggered and there's
        // nothing that we can do to avoid it.
        glGetErrors();
#   endif

    // Insert HUD overlay code here

    // Tell the compositor to begin work immediately instead of waiting for 
    // the next WaitGetPoses() call
    vr::VRCompositor()->PostPresentHandoff();
    debugAssertGLOk();
    END_PROFILER_EVENT();
}


void OpenVR::OpenVRHMD::getViewCameraMatrices
   (float                            nearPlaneZ, 
    float                            farPlaneZ,
    CFrame*                          viewToHead,
    Projection*                      viewProjection) const {

    assert((nearPlaneZ < 0.0f) && (farPlaneZ < nearPlaneZ));
    
    viewToHead[0] = toCFrame(m_xr->m_system->GetEyeToHeadTransform(vr::Eye_Left));
    viewToHead[1] = toCFrame(m_xr->m_system->GetEyeToHeadTransform(vr::Eye_Right));
    
    // Vive's near plane code is off by a factor of 2 according to G3D
    const vr::HmdMatrix44_t& ltProj = m_xr->m_system->GetProjectionMatrix(vr::Eye_Left,  -nearPlaneZ * 2.0f, -farPlaneZ);
    const vr::HmdMatrix44_t& rtProj = m_xr->m_system->GetProjectionMatrix(vr::Eye_Right, -nearPlaneZ * 2.0f, -farPlaneZ);

    Matrix4 L, R;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            L[r][c] = ltProj.m[r][c];
            R[r][c] = rtProj.m[r][c];
        }
    }

    Vector2uint32 res[2];
    getResolution(res);
    viewProjection[0] = Projection(L, Vector2(float(res[0].x), float(res[0].y)));
    viewProjection[1] = Projection(R, Vector2(float(res[1].x), float(res[1].y)));

#   if defined(G3D_DEBUG) && 0
    {
        double left, right, bottom, top, nearval, farval;
        float updirection = -1.0f;
        Matrix4(ltProjectionMatrixRowMajor4x4).getPerspectiveProjectionParameters(left, right, bottom, top, nearval, farval, updirection);
        debugAssert(fabs(-nearval - nearPlaneZ) < 0.002f);
    }
#   endif // G3D_DEBUG

}


void OpenVR::OpenVRHMD::getPassThroughVideo
    (shared_ptr<Texture>&      left,
     shared_ptr<Texture>&      right) const {

    // Returns a pointer to an internal camera object, initialized on first call.
    vr::IVRTrackedCamera* VRTrackedCamera = vr::VRTrackedCamera();
    alwaysAssertM(notNull(VRTrackedCamera), "No pass-through camera detected.");

    //Get camera handle.
    if (!m_TrackedCameraHandle) { 
        VRTrackedCamera->AcquireVideoStreamingService(vr::k_unTrackedDeviceIndex_Hmd, &m_TrackedCameraHandle);
    }

    vr::glUInt_t pglTextureID;
    vr::CameraVideoStreamFrameHeader_t pFrameHeader;
    VRTrackedCamera->GetVideoStreamTextureGL(m_TrackedCameraHandle, vr::VRTrackedCameraFrameType_Distorted, &pglTextureID, &pFrameHeader, 0);
    
    Texture::Encoding e;
    e.format = ImageFormat::RGB8I();

    left = Texture::fromGLTexture("cameraView", pglTextureID, e, AlphaFilter::ONE);

    //Currently, only monocular view.
    right = left;

}


float OpenVR::OpenVRHMD::displayFrequency() const {
    return m_xr->m_displayFrequency;
}


OpenVR::OpenVRHMD::~OpenVRHMD() {
    if (m_TrackedCameraHandle) {
        vr::VRTrackedCamera()->ReleaseVideoStreamingService(m_TrackedCameraHandle);
    }
}


void OpenVR::OpenVRHMD::getResolution(Vector2uint32* res) const {
    res[0] = res[1] = m_xr->m_resolution;
}

/////////////////////////////////////////////////////////////////////////

void OpenVR::updateTrackingData() {
    // This implementation assumes that the controller role changes at most
    // once from invalid to left or right, but never changes between those.
    // OpenVR controllers actually can change roles at runtime, so this is not
    // currently robust.

    vr::VRCompositor()->WaitGetPoses(m_trackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
    
    for (int d = 0; d < vr::k_unMaxTrackedDeviceCount; ++d) {
        const CFrame& frame = toCFrame(m_trackedDevicePose[d].mDeviceToAbsoluteTracking);
        switch (m_system->GetTrackedDeviceClass(d)) {
        case vr::TrackedDeviceClass_HMD:
            if (isNull(m_hmd)) {
                m_hmd = OpenVRHMD::create(m_objectArray.size(), d, "XR Head", this);
                setFrame(m_hmd, frame, frame);
                m_objectArray.append(m_hmd);
                m_hmdArray.append(m_hmd);
                m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, m_hmd));

                // Create the hands
                m_hmd->m_rightHand = Hand::create(m_objectArray.size(), -1, "XR Right Hand");
                m_objectArray.append(m_hmd->m_rightHand);
                m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, m_hmd->m_rightHand));

                m_hmd->m_leftHand = Hand::create(m_objectArray.size(), -1, "XR Left Hand");
                m_objectArray.append(m_hmd->m_leftHand);
                m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, m_hmd->m_leftHand));
            } else {
                setFrame(m_hmd, frame, m_hmd->frame());
            }
            break;

        case vr::TrackedDeviceClass_GenericTracker: 
        case vr::TrackedDeviceClass_TrackingReference:
            {
                // See if this object is already known to the system
                bool justDiscovered = true;
                for (const shared_ptr<Object>& object : m_objectArray) {
                    if (object->nativeAPIIndex() == d) {
                        // This controller is known, update it
                        setFrame(object, frame, object->frame());
                        justDiscovered = false;
                        break;
                    }
                }

                if (justDiscovered) {
                    // This is a new tracked object, instantiate it
                    const String& name = (m_system->GetTrackedDeviceClass(d) == vr::TrackedDeviceClass_TrackingReference) ?
                        format("XR Tracking Reference %d", d) :
                        format("XR Object %d", d);

                    const shared_ptr<Object>& object = Object::create(int(m_objectArray.size()), d, name);
                    setFrame(object, frame, frame);
                    m_objectArray.append(object);
                    m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, object));
                }
            } // tracker
            break;

        case vr::TrackedDeviceClass_Controller:
            {
                const vr::ETrackedControllerRole role = m_system->GetControllerRoleForTrackedDeviceIndex(d);

                if (role != vr::TrackedControllerRole_Invalid) {
                    // See if this controller is already known to the system
                    bool justDiscovered = true;
                    for (const shared_ptr<OpenVRController>& controller : m_controllerArray) {
                        if (controller->nativeAPIIndex() == d) {
                            // This controller is known, update it
                            setFrame(controller, frame, controller->frame());
                            justDiscovered = false;
                            break;
                        }
                    }

                    if (justDiscovered) {
                        // This is a new controller, instantiate it

                        const bool isVive = beginsWith(toLower(getHMDString(m_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String)), "vive");
                        String modelFilename;
                        if (isVive) {
                            modelFilename = System::findDataFile("model/vr/vive_1.5_controller.ArticulatedModel.Any");
                        } else {
                            modelFilename = System::findDataFile((role == vr::TrackedControllerRole_RightHand) ? "model/vr/rift_cv1_right_controller.ArticulatedModel.Any" : "model/vr/rift_cv1_left_controller.ArticulatedModel.Any");
                        }

                        const shared_ptr<OpenVRController>& controller = OpenVRController::create(int(m_objectArray.size()), d, 
                            (role == vr::TrackedControllerRole_LeftHand) ? "XR Left Controller" : "XR Right Controller",
                            (role == vr::TrackedControllerRole_RightHand),
                            m_hasTouchpad,
                            modelFilename);
                        debugAssert(role >= 1 && role <= 2);
                        setFrame(controller, frame, frame);
                        m_objectArray.append(controller);
                        m_controllerArray.append(controller);
                        m_eventQueue.pushBack(XR::Event::create(XR::Event::OBJECT_CREATED, controller));

                        if (role == vr::TrackedControllerRole_RightHand) {
                            setRightController(controller);
                        } else if (role == vr::TrackedControllerRole_LeftHand) {
                            setLeftController(controller);
                        } // role
                    } // if just discovered
                } // if has role
            } // controller
            break;

        default:;
        } // switch
    } // for

    // Update controller button events
    for (const shared_ptr<OpenVRController>& controller : m_controllerArray) {
        alwaysAssertM(notNull(controller), "Null controller");
        controller->update(m_system);
    }

    static const CFrame leftControllerToHand =
        CFrame::fromXYZYPRDegrees(-0.02f * units::meters(), -0.02f * units::meters(), 0.13f * units::meters(),-2.0f, -23.0f);
    static const CFrame rightControllerToHand =
        CFrame::fromXYZYPRDegrees(0.02f * units::meters(), -0.02f * units::meters(), 0.13f * units::meters(), -2.0f, -23.0f);

    // Update the inferred hands from the controllers
    if (m_leftController) {
        setFrame(m_hmd->m_leftHand, m_leftController->frame() * leftControllerToHand, m_hmd->m_leftHand->previousFrame());
    }

    if (m_rightController) {
        setFrame(m_hmd->m_rightHand, m_rightController->frame() * rightControllerToHand, m_hmd->m_rightHand->previousFrame());
    }


    if (false) {
        // Debugging code for events
        vr::VREvent_t vrEvent;
        while (m_system->PollNextEvent(&vrEvent, sizeof(vrEvent))) {
            switch (vrEvent.eventType) {            
            // Button 32 = k_EButton_Axis0 is the large DPad/touch button on the Vive.
            case vr::VREvent_ButtonPress:
                debugPrintf("Device %d sent button %d press\n", vrEvent.trackedDeviceIndex, vrEvent.data.controller.button);
                break;

            case vr::VREvent_ButtonUnpress:
                debugPrintf("Device %d sent button %d unpress\n", vrEvent.trackedDeviceIndex, vrEvent.data.controller.button);
                break;

            default:
                // Ignore event
                ;
            }
        } 
    }
#if 0
        fprintf(stderr, "Devices tracked this frame: \n");
        int poseCount = 0;
	    for (int d = 0; d < vr::k_unMaxTrackedDeviceCount; ++d)	{
		    if (trackedDevicePose[d].bPoseIsValid) {
			    ++poseCount;
			    switch (hmd->GetTrackedDeviceClass(d)) {
                case vr::TrackedDeviceClass_Controller:        fprintf(stderr, "   Controller: ["); break;
                case vr::TrackedDeviceClass_HMD:               fprintf(stderr, "   HMD: ["); break;
                case vr::TrackedDeviceClass_Invalid:           fprintf(stderr, "   <invalid>: ["); break;
                case vr::TrackedDeviceClass_Other:             fprintf(stderr, "   Other: ["); break;
                case vr::TrackedDeviceClass_GenericTracke:	   fprintf(stderr, "   Generic: ["); break;
                case vr::TrackedDeviceClass_TrackingReference: fprintf(stderr, "   Reference: ["); break;
                default:                                       fprintf(stderr, "   ???: ["); break;
			    }
                for (int r = 0; r < 3; ++r) { 
                    for (int c = 0; c < 4; ++c) {
                        fprintf(stderr, "%g, ", trackedDevicePose[d].mDeviceToAbsoluteTracking.m[r][c]);
                    }
                }
                fprintf(stderr, "]\n");
		    }
	    }
        fprintf(stderr, "\n");
#   endif
}


void OpenVR::setLeftController(const shared_ptr<OpenVRController>& controller) {
    m_leftController = controller;
    m_hmd->m_leftController = controller;
}


void OpenVR::setRightController(const shared_ptr<OpenVRController>& controller){
    m_rightController = controller;
    m_hmd->m_rightController = controller;
}


void OpenVR::preGraphicsInit(const XR::Settings& settings) {
    m_resolution = Vector2uint32(1024, 1024);
    
    ////////////////////////////////////////////////////////////
    // Initialize OpenVR

    try {
        vr::EVRInitError eError = vr::VRInitError_None;
        m_system = vr::VR_Init(&eError, vr::VRApplication_Scene);

        if (isNull(m_system)) {
            // Initialization failed with no fallback
            throw String("No HMD");
        } else {
            if (eError != vr::VRInitError_None) {
                throw vr::VR_GetVRInitErrorAsEnglishDescription(eError);
            }
            
            // get the proper resolution of the hmd
            m_system->GetRecommendedRenderTargetSize(&m_resolution.x, &m_resolution.y);
            
            const String& driver = getHMDString(m_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
            const String& model  = getHMDString(m_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ModelNumber_String);
            const String& serial = getHMDString(m_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
            const float   freq   = getHMDFloat(m_system,  vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
            const String& manufacturer = getHMDString(m_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_ManufacturerName_String);
            m_hasTouchpad = beginsWith(toLower(model), "vive") || beginsWith(toLower(driver), "holographic");
            logLazyPrintf("G3D::OpenVRHMD::m_system Description:\n  driver='%s'\n  manufacturer='%s'\n  model='%s'\n  serial='%s'\n  preferred resolution = %d x %d\n  framerate=%g Hz\n  hasTouchpad=%s\n\n", driver.c_str(), manufacturer.c_str(), model.c_str(), serial.c_str(), m_resolution.x, m_resolution.y, freq, m_hasTouchpad ? "true" : "false");

            // Disable supersampling. These "Native" resolutions have already been padded
            // by SteamVR to include a guard band for the timewarp algorithm, so they give
            // 1:1 pixels but are rendering more pixels than the display actually has.
            if (toLower(model).find("vive_pro") != String::npos) {
                const Vector2uint32 native(2016, 2240);
                if (m_resolution.x > native.x || m_resolution.y > native.y) {
                    logLazyPrintf("OpenVR::preGraphicsInit: Detected supersampling by SteamVR on Vive Pro. Lowering resolution to %d x %d\n\n", native.x, native.y);
                    m_resolution = native;
                }
            } else if (toLower(model).find("rift cv1") != String::npos) {
                const Vector2uint32 native(1334, 1600);
                if (m_resolution.x > native.x || m_resolution.y > native.y) {
                    logLazyPrintf("OpenVR::preGraphicsInit: Detected supersampling by SteamVR on Oculus Rift. Lowering resolution to %d x %d\n\n", native.x, native.y);
                    m_resolution = native;
                }
            } else if (toLower(model).find("vive mv") != String::npos) {
                const Vector2uint32 native(1512, 1680);
                if (m_resolution.x > native.x || m_resolution.y > native.y) {
                    logLazyPrintf("OpenVR::preGraphicsInit: Detected supersampling by SteamVR on Vive. Lowering resolution to %d x %d\n\n", native.x, native.y);
                    m_resolution = native;
                }
            }

            // Initialize the compositor
            vr::IVRCompositor* compositor = vr::VRCompositor();
            if (! compositor) {
                vr::VR_Shutdown();
                m_system = nullptr;
                throw "OpenVR Compositor initialization failed. See log file for details\n";
            }
        }
    } catch (const String& error) {
        logLazyPrintf("OpenVR Initialization Error: %s\n", error.c_str());
        throw error;
    }
}


String OpenVR::OpenVRController::modelFilename() const {
    return m_modelFilename;
}



void OpenVR::postGraphicsInit(const XR::Settings& settings)  {
    m_displayFrequency = getHMDFloat(m_system, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_DisplayFrequency_Float);
}


void OpenVR::cleanup() {
    vr::VR_Shutdown();
    m_system = nullptr;
}


bool OpenVR::available() {
    return vr::VR_IsHmdPresent() && vr::VR_IsRuntimeInstalled();
}


} // namespace G3D
#endif
