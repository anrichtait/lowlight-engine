/**
  \file G3D-app.lib/include/G3D-app/VRApp.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_VRApp_h

#include "G3D-base/platform.h"
#include "G3D-gfx/Texture.h"
#include "G3D-app/GApp.h"

namespace G3D {

class MarkerEntity;
class XR;
class XRWidget;

/** \brief Application framework for HMD Virtual Reality programs on HTC Vive, Oculus Rift, and Oculus DK2.

    Use the tab key to toggle seeing the GUI in the HMD.
    
    For many programs, simply changing from inheriting your App from GApp to VRApp will immediately
    add HMD support. You must have the OpenVR Runtime (AKA SteamVR) installed to use VRApp. OpenVR is 
    a free download as part of (Steam)[https://steamcdn-a.akamaihd.net/client/installer/SteamSetup.exe], which
    is also free download for multiple platforms.
    
    You do *not* need to install the Oculus, SteamVR, or OpenVR SDKs--G3D includes the files that you need.

    There are several MarkerEntity and reference frames at play for VR in order to attempt to model
    the degrees of freedom for a tracked HMD while still minimizing the amount of code needed to make
    a non-VR GApp subclass run as a VRGApp subclass. This API is in a very early state and subject to 
    continous change.

    The reference frames are:
- World
  - "XR Tracked Volume" [physical room. Can be changed by teleportation/explicit simulation based motion]
    - "XR Head" [determined by tracking]
      - "XR Left Eye" [fixed for each device]
      - "XR Right Eye" [fixed for each device]
    - Other "XR Tracked Object"s [determined by tracking]

Beware that this has some important implications:
 - If you want a body frame, it must be inferred from the hands & head (or itself tracked with a separate real-world object)
 - There is nothing inherent in the scene graph constraining the head and hands to have a plausible distance relationship. App-specific logic must enforce those constraints.
 - The "default camera" from a non-VR scene becomes the default TrackedVolume coordinate frame. It does not become the head.
 - The TrackedVolume must be moved vertically in world space based on the user’s height and standing/seating status
 - Movement of the TrackedVolume must be constrained by physics & collisions with geometry attached to the head frame in the world (not based on the TrackedVolume itself)


 Supports the following extra fields on G3D::Scene in a data file for automatically adding
 G3D::VisibleEntity instances with appropriate geometry that track the relevant MarkerEntity
 created by XRWidget:

 ``````````````````````````````````````````````````````
 vr = {
    avatar = {
        addHandEntity = true;
        addControllerEntity = true;
        addTorsoEntity = true;
    }
 }
 ``````````````````````````````````````````````````````

 \see XR, XRWidget
 
*/
class VRApp : public GApp {
private:

    /** Used to hack the window to a fixed size, as required by the HMD for mirroring and HUD rendering */
    static const GApp::Settings& makeFixedSize(const GApp::Settings& s);

protected:

    /** Maximum number of views ("eyes") for HMD 0 */
    static const unsigned int       MAX_VIEWS = 4;

    /** Intended for subclasses to redefine to themselves. This allows application subclasses
        to invoke `super::onInit`, etc., and easily change their base class without rewriting
        all methods. */
    typedef GApp super;
    
    /**
      The HDR framebuffer used by G3D::Film for the HMD. 
      Comparable to GApp::monitorFramebuffer.
     */
    shared_ptr<Framebuffer>         m_hmdHDRFramebuffer[MAX_VIEWS];

    /** LDR faux-"hardware framebuffer" for the HMD, comparable to GApp::m_osWindowDeviceFramebuffer.

        The m_framebuffer is still bound during the default onGraphics3D and then
        resolved by Film to the m_hmdDeviceFramebuffer.
        */
    shared_ptr<Framebuffer>         m_hmdDeviceFramebuffer[MAX_VIEWS];

    /** Per-eye */
    shared_ptr<AmbientOcclusion>    m_ambientOcclusionArray[MAX_VIEWS];

    /** Per eye Film instance for VR. onGraphics switches m_film between them. */
    shared_ptr<Film>                m_hmdFilm[2];
    
    SubmitToDisplayMode             m_vrSubmitToDisplayMode;

    /** Automatically turned on when the scene is loaded,
       disabled only if frame rate can't be maintained */
    bool                            m_highQualityWarping;

    /** The active m_gbuffer is switched between these per eye. That allows
        reprojection between them. */
    shared_ptr<GBuffer>             m_hmdGBuffer[2];

    /** \see m_numSlowFrames */
    static const int                MAX_SLOW_FRAMES = 20;

    /** The number of frames during which the renderer failed to reach the desired frame rate.
        When this count hits MAX_SLOW_FRAMES, some post effects are disabled and m_numSlowFrames
        resets.
    */
    int                             m_numSlowFrames;

    /** If true, onGraphics2D is captured and displayed in the HMD. By default, TAB toggles this. */
    float                           m_hudEnabled;

    /** Position at which onGraphics2D renders on the virtual HUD layer if m_hudEnabled == true. 
    
    \sa VRApp::Settings::hudSpace */
    CFrame                          m_hudFrame;

    /** Width in meters of the HUD layer used to display onGraphics2D content in the HMD. \sa VRApp::Settings::hudSpace */
    float                           m_hudWidth;

    /** Color of the HUD background, which reveals the boundaries of the virtual display */
    Color4                          m_hudBackgroundColor;

    /** If true, teleport the XR Tracked Volume as needed to maintain
        constant height above ground (as determined by ray casting). 
        */
    bool                            m_maintainHeightOverGround = true;
           
    shared_ptr<Texture>             m_cursorPointerTexture;

    /** If true, remove the outer part of the view that will not appear in VR.
        If you change this from the default, do so before loadScene or onGraphics is invoked */
    bool                            m_forceDiskFramebuffer = true;

    /** if m_cameraManipulator is a FirstPersonManipulator and m_trackingOverridesPitch is true,
     then zero out the pitch and roll in \a source */
    CFrame maybeRemovePitchAndRoll(const CFrame& source) const;

    /** If frame rate is being consistently missed, reduce the effects on activeCamera() */
    void maybeAdjustEffects();
    
    /** Called by maybeAdjustEffects when the frame rate is too low */
    void decreaseEffects();

    void processTeleportation(RealTime rdt);

    /** Invoked by onAfterLoadScene to optionally create avatar parts unless the scene specifies otherwise.    
        \sa Scene::VRSettings::Avatar */
    virtual void addAvatar();

    /** VR equivalent of swapBuffers() for the HMD */
    virtual void vrSubmitToDisplay();

public:
    
    /** The window will be forced to non-resizable */
    VRApp(const GApp::Settings& settings = VRApp::Settings());

    virtual ~VRApp();

    virtual void onInit() override;
    
    virtual void onBeforeSimulation(RealTime& rdt, SimTime& sdt, SimTime& idt) override;
    virtual void onAfterSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;

    /** Set up a double-eye call for onGraphics3D */
    virtual void onGraphics(RenderDevice* rd, Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) override;

    virtual void setActiveCamera(const shared_ptr<Camera>& camera) override;

    virtual void onCleanup() override;
    
    /** Intentionally empty so that subclasses don't accidentally swap buffers. Simplifies upgrading existing apps to VRApps */
    virtual void swapBuffers() override;

    virtual void onAfterLoadScene(const Any &any, const String &sceneName) override;

    /** Support for toggling the HUD using the TAB key */
    virtual bool onEvent(const GEvent& event) override;
};



} // namespace
