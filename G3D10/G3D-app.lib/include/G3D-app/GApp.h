/**
  \file G3D-app.lib/include/G3D-app/GApp.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define G3D_app_GApp_h

#include "G3D-base/platform.h"
#include "G3D-base/Stopwatch.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/NetworkDevice.h"
#include "G3D-gfx/GazeTracker.h"
#include "G3D-gfx/OSWindow.h"
#include "G3D-app/Camera.h"
#include "G3D-app/GFont.h"
#include "G3D-app/FirstPersonManipulator.h"
#include "G3D-app/Widget.h"
#include "G3D-app/GConsole.h"
#include "G3D-app/DeveloperWindow.h"
#include "G3D-app/Film.h"
#include "G3D-app/Scene.h"
#include "G3D-app/GBuffer.h"
#include "G3D-app/debugDraw.h"
#include "G3D-app/ArticulatedModelSpecificationEditorDialog.h"
#include "G3D-app/DefaultRenderer.h"
#include <mutex>

namespace G3D {

// forward declare heavily dependent classes
class DebugTextWidget;
class DepthOfField;
class Log;
class MotionBlur;
class RenderDevice;
class Renderer;
class Scene;
class ScreenCapture;
class Shape;
class UserInput;
class XR;
class MarkerEntity;
class XRWidget;


/**
 \brief Optional base class for quickly creating 3D applications.

 GApp has several event handlers implemented as virtual methods.  It invokes these in
 a cooperative, round-robin fashion.  This avoids the need for threads in most
 applications.  The methods are, in order of invocation from GApp::oneFrame:

 <ul>
 <li> GApp::onEvent - invoked once for each G3D::GEvent
 <li> GApp::onAfterEvents - latch any polled state before onUserInput processing
 <li> GApp::onUserInput - process the current state of the keyboard, mouse, and game pads
 <li> GApp::onNetwork - receive network packets; network <i>send</i> occurs wherever it is needed
 <li> GApp::onAI - game logic and NPC AI
 <li> GApp::onSimulation - physical simulation
 <li> GApp::onPose - create arrays of Surface and Surface2D for rendering
 <li> GApp::onWait - tasks to process while waiting for the next frame to start (when there is a refresh limiter)
 <li> GApp::onGraphics - render the Surface and Surface2D arrays.  By default, this invokes two helper methods:
   <ul>
    <li> GApp::onGraphics3D - render the Surface array and any immediate mode 3D
    <li> GApp::onGraphics2D - render the Surface2D array and any immediate mode 2D
   </ul>
 </ul>

 To customize the rendering algorithm without modifying the post-processing
 setup, you can use the default GApp::onGraphics3D and simply
 change the GApp::m_renderer by subclassing G3D::Renderer.

 The GApp::run method starts the main loop.  It invokes GApp::onInit, runs the main loop
 until completion, and then invokes GApp::onCleanup.

 onWait runs before onGraphics because the beginning of onGraphics causes the CPU to block, waiting for the GPU
 to complete the previous frame.

 When you override a method, invoke the GApp version of that method to ensure that Widget%s still work
 properly.  This allows you to control whether your per-app operations occur before or after the Widget ones.

 There are a number of framebuffers:

 - GApp::m_deviceFramebuffer is a pointer to the current display buffer. This is GApp::m_osWindowDeviceFramebuffer when using a monitor and VRApp::m_hmdDeviceFramebuffer for a HMD.
 - GApp::m_osWindowDeviceFramebuffer is the actual buffer of the display. For a monitor this is the "hardware framebuffer" with OpenGL ID 0, which has special support in G3D::Framebuffer.
 - GApp::m_framebuffer, GApp::m_depthPeelFramebuffer, GApp::m_gbuffer point to the framebuffer, depth-peeling framebuffer, and GBuffer that GApp::onGraphics3D should render into. The resolution may vary from m_osWindowHDRFramebuffer for super or sub sampling. This may be rebound between onGraphics3D calls by GApp or subclasses such as VRApp. 
 - GApp::m_osWindowHDRFramebuffer and GApp::m_osWindowGBuffer are a software high dynamic range framebuffer and GBuffer sized for the OSWindow's bounds, plus any guard band padding and scaled by GApp::HDRFramebufferSettings::sampleRateOneDimension. By default, GApp binds GApp::m_framebuffer and GApp::m_gbuffer to these before invoking onGraphics3D.
 - VRApp::m_hmdDeviceFramebuffer is an array of per-eye HMD backing LDR framebuffers (analogous to m_osWindowDeviceFramebuffer)
 - VRApp::m_hmdHDRFramebuffer and VRApp::m_hmdGBuffer are arrays of per-eye (analogous to GApp::m_osWindowHDRFramebuffer and GApp::m_osWindowGBuffer), which VRApp binds m_framebuffer and m_gbuffer for onGraphics3D.

 \sa GApp::Settings, OSWindow, RenderDevice, G3D_START_AT_MAIN
*/
class GApp {
public:
    friend class OSWindow;
    friend class DeveloperWindow;
    friend class SceneEditorWindow;
    friend class Shader;
    friend class DebugTextWidget;

    // See documentation on setSubmitToDisplayMode
    // Note that this is also used for the VR API
    G3D_DECLARE_ENUM_CLASS(SubmitToDisplayMode,
        EXPLICIT,
        MAXIMIZE_THROUGHPUT,
        BALANCE,
        MINIMIZE_LATENCY);

    // See documentation on setSubmitToDisplayMode
    G3D_DECLARE_ENUM_CLASS(DebugVRMirrorMode,
        NONE,
        
        /** Both eyes without HMD distortion to correct for chromatic abberation. This is the output of onGraphics3D */
        BOTH_EYES,
        
        /** Right eye only, cropped to fit the screen */
        RIGHT_EYE_CROP,
        
        RIGHT_EYE_FULL);

    class Settings {
    public:
        OSWindow::Settings       window;

        /**
           If "<AUTO>", will be set to the directory in which the executable resides.
           This is used to invoke System::setDataDir()
        */
        String                  dataDir;

        /**
          Empty by default. Used to specify additional data directories for projects that have multiple.
        */
        std::vector<String>     dataDirs;

        /**
           Can be relative to the G3D data directory (e.g. "font/dominant.fnt")
           or relative to the current directory.
           Default is "console-small.fnt"
        */
        String                  debugFontName;

        String                  logFilename;

        /** If true, the G3D::DeveleloperWindow and G3D::CameraControlWindow will be enabled and
            accessible by pushing F12.
            These require osx.gtm, arial.fnt, greek.fnt, and icon.fnt to be in locations where
            System::findDataFile can locate them (the program working directory is one such location).
        */
        bool                    useDeveloperTools;

        /* Default is "arial.fnt". \sa GFont */
        String                  developerToolsFontName;

        /* Default is "osx-10.7.gtm" \sa GuiTheme */
        String                  developerToolsThemeName;

        /**
            When true, GAapp ensures that g3d-license.txt exists in the current
            directory.  That file is written from the return value of G3D::license() */
        bool                    writeLicenseFile;

        /** These are not necessarily followed if not using the DefaultRenderer */
        class RendererSettings {
        public:
            /** Function pointer for creating an instance of the renderer. By default, this is &(DefaultRenderer::create).*/
            shared_ptr<class Renderer> (*factory)();
            bool                deferredShading;
            bool                orderIndependentTransparency;
            RendererSettings();
        } renderer;

        class HDRFramebufferSettings {
        public:

            /** Size of G3D::GApp::m_osWindowHDRFramebuffer (and G3D::GApp::m_gbuffer, if used) in each dimension as a
                multiple of the size of G3D::GApp::m_osWindowDeviceFramebuffer in that dimension, not including the colorGuardBandThickness
                and depthGuardBandThickness.

                Default is 1.0. Values greater than 1 result in supersampling (e.g., 
                sampleRateOneDimension = 2 produces 4x FSAA), values less than 1 result in subsampling.
                The result will always be stretched to fill the screen.
            */
            float                       sampleRateOneDimension = 1.0f;

            /** Formats to attempt to use for the Film, in order of decreasing preference */
            Array<const ImageFormat*>   preferredColorFormats;

            /** Formats to attempt to use for the Film, in order of decreasing preference.
               nullptr (or an empty list) indicates that no depth buffer should be allocated.

               If you want separate depth and stencil attachments, you must explicitly allocate
               the stencil buffer yourself and attach it to the depth buffer.
              */
            Array<const ImageFormat*>   preferredDepthFormats;
            
            /**
                The default call to Film::exposeAndRender in the sample "starter" project crops these off, and the default
                App::onGraphics3D in that project.

                The use of a guard band allows screen-space effects to avoid boundary cases at the edge of the screen. For example,
                G3D::AmbientOcclusion, G3D::MotionBlur, and G3D::DepthOfField.

                Guard band pixels count against the field of view (this keeps rendering and culling code simpler), so the effective
                field of view observed.

                Note that a 128-pixel guard band at 1920x1080 allocates 40% more pixels than no guard band, so there may be a substantial
                memory overhead to a guard band even though there is little per-pixel rendering cost due to using
                RenderDevice::clip2D.

                Must be non-negative. Default value is (0, 0).
                These are final frame pixels, so when changing sampleRateOneDimension, it is usually a good
                idea to change these values by the same amount.

                \image html guardBand.png
                */
            Vector2int16            colorGuardBandThickness;

            /**
                Must be non-negative and at least as large as colorGuardBandThickness. Default value is (0, 0).
                These are final frame pixels, so when changing sampleRateOneDimension, it is usually a good 
                idea to change these values by the same amount.
                \image html guardBand.png
            */
            Vector2int16            depthGuardBandThickness;

            HDRFramebufferSettings() {
                preferredColorFormats.append(ImageFormat::R11G11B10F(), ImageFormat::RGB16F(), ImageFormat::RGBA16F(), ImageFormat::RGB32F(), ImageFormat::RGBA32F(), ImageFormat::RGBA8());
                preferredDepthFormats.append(ImageFormat::DEPTH32F(), ImageFormat::DEPTH32(), ImageFormat::DEPTH24());
            }

            void setGuardBandsAndSampleRate(int colorGuard, int extraDepthGuard = 0, float sampleRate = 1.0f) {
                sampleRateOneDimension = sampleRate;
                colorGuardBandThickness = Vector2int16(Vector2(float(colorGuard), float(colorGuard)) * sampleRateOneDimension);
                depthGuardBandThickness = Vector2int16(Vector2(float(extraDepthGuard), float(extraDepthGuard)) * sampleRateOneDimension) + colorGuardBandThickness;
            }

            Vector2int32 hdrFramebufferSizeFromDeviceSize(const Vector2int32 osWindowSize) const;

            Vector2int16 trimBandThickness() const {
                return depthGuardBandThickness - colorGuardBandThickness;
            }
        };

        HDRFramebufferSettings  hdrFramebuffer;

        /** Arguments to the program, from argv.  The first is the name of the program. */
        Array<String>           argArray;

        class ScreenCaptureSettings {
        public:
            /** Directory in which all screen captures (screenshots, video) are saved. Defaults to current directory. */
            String              outputDirectory;

            /** Prefix added to all capture filenames. Defaults to the application name. 
                Regardless of this prefix, the date and a unique integer will always be appended. */
            String              filenamePrefix;

            bool                includeG3DRevision = false;

            /** Include the version number of the project in the filename.
            
                Defaults to false. Can be changed in app settings, but will 
                be forced to false and disabled if SCM command-line tools are 
                not present or the \a outputDirectory is not under version control. */
            bool                includeAppRevision = false;

            /** When true, capture journal entries will be allowed to add to source control.

                Currently supports Subversion (svn) and Git (git).
                Will not trigger a commit or push.

                Defaults to true, but will be forced to false and
                disabled if SCM command-line tools are not present or
                the \a outputDirectory is not under version control. */
            bool                addFilesToSourceControl = true;

            ScreenCaptureSettings();
        };

        ScreenCaptureSettings   screenCapture;

           
        class VR {
        public:

            /** Defaults to false. Cannot be changed once VRApp is initialized. */
            DebugVRMirrorMode   debugMirrorMode;

            /** If no HMD is present, should the system provide a virtual HMD for development
                and debugging? If false and there
                is no HMD, the system will throw an error. */
            bool                emulateHMDIfMissing;

            /** If no physical VR controller is present, should the system provide a
                virtual one that is locked relative to the HMD? This is useful for
                both development and deployment.
                
                If false and there is no physical controller, then there is no error--
                tracking will simply report nothing. */
            bool                emulateControllerIfMissing;

            /** Use pitch control from the HMD instead of from the m_cameraManipulator. Defaults to true.
            For walking simulators, we recommend m_trackingOverridesPitch = true.
            For driving or flight simulators, we recommend m_trackingOverridesPitch = false.

            Yaw control is not overriden in order to allow typical first-person strafing movement and rotation.
            (We may provide an option to do so, or at least to compose them, in the future). Beware that this can be confusing
            to the user unless some kind of body avatar is rendered.

            Can be changed at runtime, although some inconsistency may occur for a few frames after the change.
            */
            bool                trackingOverridesPitch;

            /** If this is true, after too many frames have rendered below the target frame rate
            post-processing effects will be selectively disabled on the active camera. Defaults
            to true.
            */
            bool                disablePostEffectsIfTooSlow;

            /**
             Force motionBlurSettings on VR eye cameras at render time.
            */
            bool                overrideMotionBlur;
            
            /** Defaults to 100% camera motion, 15% exposure time, enabled. 
                \sa overrideMotionBlur */
            MotionBlurSettings  motionBlurSettings;

            /**
             Force depthOfFieldSettings on VR eye cameras at render time.
            */
            bool                overrideDepthOfField;

            /** Disabled */
            DepthOfFieldSettings depthOfFieldSettings;

            /** Must be CAMERA (player head), OBJECT (player body, the default), or WORLD (fixed at the origin) */
            FrameName           hudSpace;

            shared_ptr<XR>      xrSystem;

            VR(bool debugMirrorMode = DebugVRMirrorMode::NONE) : 
                debugMirrorMode(debugMirrorMode), 
                emulateHMDIfMissing(true),
                emulateControllerIfMissing(true),
                trackingOverridesPitch(true),
                disablePostEffectsIfTooSlow(true),
                overrideMotionBlur(true),
                overrideDepthOfField(true),
                hudSpace(FrameName::OBJECT) {

                motionBlurSettings.setExposureFraction(0.15f);
                motionBlurSettings.setEnabled(true);

                depthOfFieldSettings.setEnabled(false);
            }
        } vr;
        /** Also invokes initGLG3D() */
        Settings();

        /** Also invokes initGLG3D() */
        Settings(int argc, const char* argv[]);

        virtual ~Settings() {}
    };

    class DebugShape {
    public:
        shared_ptr<Shape>   shape;
        Color4              solidColor;
        Color4              wireColor;
        CoordinateFrame     frame;
        DebugID             id;
        /** Clear after this time (always draw before clearing) */
        RealTime            endTime;
    };

    class DebugLabel {
    public:
        Point3 wsPos;
        GuiText text;
        DebugID id;
        GFont::XAlign xalign;
        GFont::YAlign yalign;
        float size;
        RealTime endTime;
    };
    
    /** Last DebugShape::id issued */
    DebugID              m_lastDebugID;

    /** Defaults to a cyclops EmulatedGazeTracker. Set to a binocular EmulatedGazeTracker for
        a slightly slower but much better binocular simulation (tip: turn on Modle::accelerated ray casts for that),
        or instantiate a real gaze tracker if you have one.*/
    shared_ptr<GazeTracker> m_gazeTracker;

    /** Gaze for each eye for the current frame */
    GazeTracker::Gaze               m_gazeArray[2];

    /** Gaze vector for the current eye, relative to the activeCamera(). This is a pointer into m_gazeArray  */
    GazeTracker::Gaze*              m_gaze;

    /** \brief Shapes to be rendered each frame.

        Added to by G3D::debugDraw.
        Rendered by drawDebugShapes();
        Automatically cleared once per frame.
      */
    Array<DebugShape>    debugShapeArray;

    /** Labels to be rendered each frame, updated at the same times as debugShapeArray */
    Array<DebugLabel>    debugLabelArray;

    /** \brief Draw everything in debugShapeArray.

        Subclasses should call from onGraphics3D() or onGraphics().
        This will sort the debugShapeArray from back to front
        according to the current camera.

        \sa debugDraw, Shape, DebugID, removeAllDebugShapes, removeDebugShape
     */
    virtual void drawDebugShapes();

    /**
        \brief Clears all debug shapes, regardless of their pending display time.

        \sa debugDraw, Shape, DebugID, removeDebugShape, drawDebugShapes
    */
    void removeAllDebugShapes();

    /**
        \brief Clears just this debug shape (if it exists), regardless of its pending display time.

        \sa debugDraw, Shape, DebugID, removeAllDebugShapes, drawDebugShapes
    */
    void removeDebugShape(DebugID id);

private:

    /** Called from init. */
    shared_ptr<GFont> loadFont(const String& fontName);

    ScreenCapture*                  m_screenCapture;

    OSWindow*                       m_window;

    bool                            m_hasUserCreatedWindow;
    bool                            m_hasUserCreatedRenderDevice;

    shared_ptr<Scene>               m_scene;

    SubmitToDisplayMode             m_submitToDisplayMode;

protected:

    /** The low-level XR API. VRApp mostly communicates through an XRWidget that
        wraps this, but needs the underlying system for initialization and cleanup. */
    shared_ptr<XR>                  m_xrSystem;
    shared_ptr<XRWidget>            m_xrWidget;

    Stopwatch                       m_graphicsWatch;
    Stopwatch                       m_poseWatch;
    Stopwatch                       m_logicWatch;
    Stopwatch                       m_networkWatch;
    Stopwatch                       m_userInputWatch;
    Stopwatch                       m_simulationWatch;
    Stopwatch                       m_waitWatch;

    /** The original settings */
    Settings                        m_settings;

    /** onPose(), onGraphics(), and onWait() execute once every m_renderPeriod
        simulation frames. This allows UI/network/simulation to be clocked much faster
        than rendering to increase responsiveness. */
    int                             m_renderPeriod;

    shared_ptr<WidgetManager>       m_widgetManager;

    bool                            m_endProgram;
    int                             m_exitCode;

    /**
       Used to find the frame for defaultCamera.
    */
    shared_ptr<Manipulator>         m_cameraManipulator;

    std::mutex                      m_debugTextMutex;

    /** Used by the default onGraphics3D to render Surface%s. */
    shared_ptr<Renderer>            m_renderer;

    /**
       Strings that have been printed with screenPrintf.
       Protected by m_debugTextMutex.
    */
    Array<String>                   debugText;

    Color4                          m_debugTextColor;
    Color4                          m_debugTextOutlineColor;

    shared_ptr<DebugTextWidget>     m_debugTextWidget;

    /** Set by onGraphics for each onGraphics3D call in VRApp. Always 0 
        in GApp. 
        */
    int                             m_currentEyeIndex;

    /** Allocated if GApp::Settings::FilmSettings::enabled was true
        when the constructor executed.  Automatically resized by
        resize() when the screen size changes.
    */
    shared_ptr<Film>                m_film;

    GBuffer::Specification          m_gbufferSpecification;

    /** The current device [LDR] framebuffer. This can be changed by GApp or VRApp just before invoking onGraphics3D. The default implementation of GApp sets it to
        m_osWindowDeviceFramebuffer. */
    shared_ptr<Framebuffer>         m_deviceFramebuffer;

    /** Bound to the current GBuffer, which is m_osWindowGBuffer by default.
       \sa m_gbufferSpecification */
    shared_ptr<GBuffer>             m_gbuffer;

    shared_ptr<DepthOfField>        m_depthOfField;

    shared_ptr<MotionBlur>          m_motionBlur;

    /** GBuffer used for the OSWindow. VRApp adds per-eye HMD GBuffers */ 
    shared_ptr<GBuffer>             m_osWindowGBuffer;

    /** Framebuffer used for rendering the 3D portion of the scene. Includes 
        a color guard band. This is then resolved to m_osWindowDeviceFramebuffer. 
        
        \see G3D::GApp::Settings::HDRFramebufferSettings
        */
    shared_ptr<Framebuffer>         m_osWindowHDRFramebuffer;

    /** The (probably low dynamic range, one sample per pixel) OpenGL hardware framebuffer for the window(). 
        Initialized in initializeOpenGL().

        \sa VRApp::m_hmdDeviceFramebuffer[] 
      */
    shared_ptr<Framebuffer>         m_osWindowDeviceFramebuffer;

    /** The framebuffer that will be used by the default onGraphics3D.  GApp binds this to m_osWindowHDRFramebuffer by default.
        VRApp binds it to VRApp::m_hmdHDRFramebuffer[VRApp::m_currentEye]. 
      */
    shared_ptr<Framebuffer>         m_framebuffer;

    shared_ptr<Framebuffer>         m_depthPeelFramebuffer;

    /** Used to track how much onWait overshot its desired target during the previous frame. */
    RealTime                        m_lastFrameOverWait;

    /** Default/current AO object for the primary view, allocated in GApp::GApp.*/
    shared_ptr<class AmbientOcclusion> m_ambientOcclusion;

    /**
       A camera that is driven by the debugController.

       This is a copy of the default camera from the scene, but is not itself in the scene.

       Do not reassign this--the CameraControlWindow is hardcoded to the original one.
    */
    shared_ptr<Camera>              m_debugCamera;

    /** Follows the activeCamera. In the Scene. */
    shared_ptr<MarkerEntity>        m_activeCameraMarker;

    /**
       Allows first person (Quake game-style) control
       using the arrow keys or W,A,S,D and the mouse.

       To disable, use:
       <pre>
       setCameraManipulator(nullptr);
       </pre>
    */
    [[deprecated]] shared_ptr<FirstPersonManipulator>  m_debugController;

    /** The currently selected camera.  \sa m_activeCameraMarker */
    shared_ptr<Camera>              m_activeCamera;

    shared_ptr<Entity>              m_activeListener;

    /** Pointer to the current GApp. GApp sets itself as current upon construction */
    static GApp*                    s_currentGApp;

    /**
      Helper for generating cube maps.  Invokes GApp::onGraphics3D six times, once for each face of a cube map.  This is convenient both for microrendering
      and for generating cube maps to later use offline.

      certain post processing effects are applied to the final image. Motion blur and depth of field are not but AO is if enabled. However AO will causes artifacts on the final image when enabled.

      \param output If empty or the first element is nullptr, this is set to a series of new reslolution x resolution ImageFormat::RGB16F() textures.  Otherwise, the provided elements are used.
      Textures are assumed to be square.  The images are generated in G3D::CubeFace order.

      \param camera the camera will have all of its parameters reset before the end of the call.

      \param depthMap Optional pre-allocated depth texture to use as the depth map when rendering each face.  Will be allocated to match the texture resolution if not provided.
      The default depth format is ImageFormat::DEPTH24().

      Example:
      \code
        Array<shared_ptr<Texture> > output;
        renderCubeMap(renderDevice, output, defaultCamera);

        const Texture::CubeMapInfo& cubeMapInfo = Texture::cubeMapInfo(CubeMapConvention::DIRECTX);
        for (int f = 0; f < 6; ++f) {
            const Texture::CubeMapInfo::Face& faceInfo = cubeMapInfo.face[f];
            shared_ptr<Image> temp = output[f]->toImage(ImageFormat::RGB8());
            temp->flipVertical();
            temp->rotate90CW(-faceInfo.rotations);
            if (faceInfo.flipY) { temp.flipVertical();   }
            if (faceInfo.flipX) { temp.flipHorizontal(); }
            temp->save(format("cube-%s.png", faceInfo.suffix.c_str()));
        }
        \endcode
    */
    virtual void renderCubeMap(RenderDevice* rd, Array<shared_ptr<Texture> >& output, const shared_ptr<Camera>& camera, const shared_ptr<Texture>& depthMap, int resolution = 1024);

    /**
       Processes all pending events on the OSWindow queue into the userInput.
       This is automatically called once per frame.  You can manually call it
       more frequently to get higher resolution mouse tracking or to prevent
       the OS from locking up (and potentially crashing) while in a lengthy
       onGraphics call.
    */
    virtual void processGEventQueue();

    static void staticConsoleCallback(const String& command, void* me);

    /** Call from onInit to create the developer HUD. */
    void createDeveloperHUD();

    virtual void setScene(const shared_ptr<Scene>& s) {
        m_scene = s;
    }


    /** Defaults to SubmitMode::MAXIMIZE_THROUGHPUT.

        SubmitToDisplayMode::EXPLICIT requires an explicit call to swapBuffers()--GApp does not perform
        swapping in this case.

        SubmitToDisplayMode::MAXIMIZE_THROUGHPUT swaps in the middle of the next frame, as soon as it needs to write
        to the hardware framebuffer to *maximize throughput* (framerate).
        This allows CPU physics, network, audio, AI, scene traversal, etc. to overlap GPU rendering,
        and even allows GPU work submission for offscreen buffers for the next frame to overlap GPU execution
        of the current frame.

        SubmitToDisplayMode::BALANCE swaps at the beginning of the next frame to *balance throughput and latency*.
        This allows CPU physics, network, audio, AI, scene traversal, etc. to overlap GPU rendering.

        SubmitToDisplayMode::MINIMIZE_LATENCY swaps at the end of the current frame to *minimize latency*. This blocks the
        CPU on the GPU until the currently-submitted work is complete.
    */
    void setSubmitToDisplayMode(SubmitToDisplayMode m) {
        m_submitToDisplayMode = m;
    }

    SubmitToDisplayMode submitToDisplayMode() const {
        return m_submitToDisplayMode;
    }

public:

    const GazeTracker::Gaze& gazeForEye(int eye) const {
        debugAssert(eye >= 0 && eye < 2);
        return m_gazeArray[eye];
    }

    /** The reference fram of the "XR Head" entity if it exists, otherwise the activeCamera frame.
        This allows unifying code across desktop and VR rendering. The VR APIs do not move the activeCamera
        with the HMD because XRWidget leaves that unmodified to represent the default tracking volume. */
    virtual CFrame headFrame() const;

    const shared_ptr<Scene>& scene() const {
        return m_scene;
    }

    template<class SceneSubclass = Scene>
    shared_ptr<SceneSubclass> typedScene() const {
        return dynamic_pointer_cast<SceneSubclass>(m_scene);
    }
    
    /** Returns a pointer to the current GApp. GApp sets itself as current upon construction */
    static GApp* current();
    
    /** Sets the current GApp; the current GApp is used for debug drawing */
    static void setCurrent(GApp* gApp);

    const shared_ptr<GazeTracker>& gazeTracker() const {
        return m_gazeTracker;
    }
    
    virtual void swapBuffers();

    /** Invoked by loadScene() after the scene has been loaded. This allows
        the GApp to modify the scene or load custom properties from the \a any
        structure.

        The scene can be accessed using the scene() method.*/
    virtual void onAfterLoadScene(const Any& any, const String& sceneName) {}

    /** Load a new scene.  A GApp may invoke this on itself, and the
        SceneEditorWindow will invoke this automatically when
        the user presses Reload or chooses a new scene from the GUI. */
    virtual void loadScene(const String& sceneName);

    /** Save the current scene over the one on disk. */
    virtual void saveScene();

    /** The currently active camera for the primary view. 
        The special G3D::MarkerEntity named "(Active Camera Marker)" follows whichever
        camera is currently active. It does not update when the camera is not in the scene.

        \sa activeListener
    */
    virtual const shared_ptr<Camera>& activeCamera() const {
        return m_activeCamera;
    }

    /** Exposes the debugging camera */
    virtual const shared_ptr<Camera>& debugCamera() const {
        return m_debugCamera;
    }

    /** The default camera is specified by the scene. Use the F2 key under the developer HUD to quickly switch to the debug camera.
        During rendering (e.g., by VRApp) the active camera may be temporarily changed. 
        
        If the scene() is not null, also creates a G3D::MarkerEntity named "activeCamera" in the scene
        that is at the position of this camera.
     */
    virtual void setActiveCamera(const shared_ptr<Camera>& camera);

    /** The default listener is the activeCamera object. Set to nullptr to disable actively
        setting the underlying AudioDevice::setListener3DAttributes every frame if you intend
        to change those explicitly in your app. 
        
        The default value is `scene()->entity("(Active Camera Marker)")`.

        \sa setActiveCamera
     */
    virtual void setActiveListener(const shared_ptr<Entity>& listener) {
        m_activeListener = listener;
    }

    /** May be nullptr */
    const shared_ptr<Entity>& activeListener() const {
        return m_activeListener;
    }

    /** Add your own debugging controls to this window.*/
    shared_ptr<GuiWindow>   debugWindow;

    /** debugWindow->pane() */
    GuiPane*                debugPane;

    virtual const SceneVisualizationSettings& sceneVisualizationSettings() const;

    const Settings& settings() const {
        return m_settings;
    }

    const shared_ptr<Renderer>& renderer() {
        return m_renderer;
    }

    void vscreenPrintf
    (const char*                 fmt,
     va_list                     argPtr) G3D_CHECK_VPRINTF_METHOD_ARGS;

    const Stopwatch& graphicsWatch() const {
        return m_graphicsWatch;
    }

    const Stopwatch& waitWatch() const {
        return m_waitWatch;
    }

    const Stopwatch& logicWatch() const {
        return m_logicWatch;
    }

    const Stopwatch& networkWatch() const {
        return m_networkWatch;
    }

    const Stopwatch& userInputWatch() const {
        return m_userInputWatch;
    }

    const Stopwatch& simulationWatch() const {
        return m_simulationWatch;
    }

    /** Initialized to GApp::Settings::dataDir, or if that is "<AUTO>",
        to  FilePath::parent(System::currentProgramFilename()). To make your program
        distributable, override the default
        and copy all data files you need to a local directory.
        Recommended setting is "data/" or "./", depending on where
        you put your data relative to the executable.

        Your data directory must contain the default debugging font,
        "console-small.fnt", unless you change it.
    */
    String                          dataDir;

    /** Initialized to empty. Used for additional data directories
        in projects that have data in multiple folders in the filesystem
        hierarchy. Use std::vector for convenience when calling 
        System::setAppDataDirs(const std::vector<String>& dataDirs).
    */
    std::vector<String>             dataDirs;

    bool dataDirsAddedToScene       = false;

    RenderDevice*                   renderDevice;

    /** Command console.
    \deprecated */
    [[deprecated]] GConsoleRef      console;

    /** The window that displays buttons for debugging.  If GApp::Settings::useDeveloperTools is true
        this will be created and added as a Widget on the GApp.  Otherwise this will be nullptr.
    */
    shared_ptr<DeveloperWindow>     developerWindow;

    /**
       nullptr if not loaded
    */
    shared_ptr<GFont>               debugFont;
    UserInput*                      userInput;

    /** Invoke to true to end the program at the end of the next event loop. */
    virtual void setExitCode(int code = 0);

    /**
       The manipulator that positions the debugCamera() every frame.
       By default, this is set to an instance of G3D::FirstPersonManipulator.  This may be
       set to <code>shared_ptr<Manipulator>()</code> to disable explicit camera positioning.

       Setting a camera manipulator automatically also adds it as a Widget if it is not already present.
       Overriding the camera manipulator automatically removes the previous manipulator as a Widget.

       Example:
       \code
        shared_ptr<UprightSplineManipulator> us = UprightSplineManipulator::create(debugCamera());
        UprightSpline spline;
        spline.extrapolationMode = SplineExtrapolationMode::CYCLIC;
        spline.append(Point3(0,0,-3));
        spline.append(Point3(1, 0, -3));
        spline.append(Point3(0,0,-3));
        spline.append(Point3(-1,0,-3));
        us->setSpline(spline);
        us->setMode(UprightSplineManipulator::Mode::PLAY_MODE);
        app->setCameraManipulator(us);
      \endcode
    */
    void setCameraManipulator(const shared_ptr<Manipulator>& man) {
        if (notNull(m_cameraManipulator)) {
            removeWidget(m_cameraManipulator);
            m_cameraManipulator.reset();
        }
        if (notNull(man)) {
            addWidget(man);
        }
        m_cameraManipulator = man;
    }

    shared_ptr<Manipulator> cameraManipulator() const {
        return m_cameraManipulator;
    }

    OSWindow* window() const {
        return m_window;
    }

    ScreenCapture* screenCapture() const {
        return m_screenCapture;
    }

    /**
       When true, screenPrintf prints to the screen.
       (default is true)
    */
    bool                    showDebugText;

    enum Action {
        ACTION_NONE,
        ACTION_QUIT,
        ACTION_SHOW_CONSOLE
    };

    /**
       When true an GKey::ESCAPE keydown event
       quits the program.
       (default is true)
    */
    Action                  escapeKeyAction;

    /**
       When true, debugTextWidget prints the frame rate and
       other data to the screen.
    */
    bool                    showRenderingStats;

    /**
       When true, the G3D::UserInput->beginEvents/endEvents processing is handled
       for you by calling processGEventQueue() before G3D::GApp::onUserInput is called.  If you turn
       this off, you must call processGEventQueue() or provide your own event to userInput processing in onUserInput.
       (default is true)
    */
    bool                    manageUserInput;

    /**
       When true, there is an assertion failure if an exception is
       thrown.

       Default is true.
    */
    bool                    catchCommonExceptions;

    /**
       @param window If null, an OSWindow will be created for you. This
       argument is useful for substituting a different window
       system (e.g., GlutWindow)

       \param createWindowOnNull Create the window or renderDevice if they are nullptr.
       Setting createWindowOnNull = false allows a subclass to explicitly decide when to invoke
       those calls.
    */
    GApp(const Settings& options = Settings(), OSWindow* window = nullptr, RenderDevice* rd = nullptr, bool createWindowOnNull = true);

    /** Called from GApp constructor to initialize OpenGL and openGL-dependent state. Can't be virtual because it is invoked from
        a constructor, but allows subclasses to perform their own pre-OpenGL steps. */
    void initializeOpenGL(RenderDevice* rd, OSWindow* window, bool createWindowIfNull, const Settings& settings);

    virtual ~GApp();

    /**
       Call this to run the app.
    */
    int run();

    /** Draw a simple, short message in the center of the screen and swap the buffers.
      Useful for loading screens and other slow operations.*/
    virtual void drawMessage(const String& message);

    /** Draws a title card */
    void drawTitle(const String& title, const String& subtitle, const Any& any, const Color3& fontColor, const Color4& backColor);

    /** Displays the texture in a new GuiWindow */
    shared_ptr<GuiWindow> show(const shared_ptr<Texture>& t, const String& windowCaption = "");

    shared_ptr<GuiWindow> show(const shared_ptr<PixelTransferBuffer>& t, const String& windowCaption = "");

    shared_ptr<GuiWindow> show(const shared_ptr<Image>& t, const String& windowCaption = "");

    /** Shows a texture by name. Convenient for creating debugging views of textures that
        are not exposed by other objects. Returns nullptr if the texture is not currently in memory. */
    shared_ptr<TextureBrowserWindow> showInTextureBrowser(const String& textureName, Rect2D rect = Rect2D::empty());
    shared_ptr<TextureBrowserWindow> showInTextureBrowser(const shared_ptr<Texture>& texture, Rect2D rect = Rect2D::empty());

    /** Returns a texture by its name, or nullptr if not found. Useful for bypassing language
        protection mechanisms when creating debugging GUIs */
    shared_ptr<Texture> textureByName(const String& name) const;

protected:

    /** Used by onWait for elapsed time. */
    RealTime               m_lastWaitTime;

    /** Seconds per frame target for the entire system \sa setFrameDuration*/
    float                  m_wallClockTargetDuration;

    /** \copydoc setLowerFrameRateInBackground */
    bool                   m_lowerFrameRateInBackground;

    /** SimTime seconds per frame, \see setFrameDuration, m_simTimeScale */
    float                  m_simTimeStep;
    float                  m_simTimeScale;
    float                  m_previousSimTimeStep;
    float                  m_previousRealTimeStep;

    RealTime               m_realTime;
    SimTime                m_simTime;

protected:

    /** Used by onSimulation for elapsed time. */
    RealTime               m_now, m_lastTime;

    Array<shared_ptr<Surface> >   m_posed3D;
    Array<shared_ptr<Surface2D> > m_posed2D;

protected:

    /** Helper for run() that actually starts the program loop. Called from run(). */
    void onRun();

    /**
       Initializes state at the beginning of onRun, including calling onCleanup.
    */
    void beginRun();

    /**
       Cleans up at the end of onRun, including calling onCleanup.
    */
    void endRun();

    /**
        A single frame of rendering, simulation, AI, events, networking,
        etc.  Invokes the onXXX methods and performs timing.
    */
    virtual void oneFrame();

    virtual void sampleGazeTrackerData();

public:

    /**
       Installs a module.  Actual insertion may be delayed until the next frame.
    */
    virtual void addWidget(const shared_ptr<Widget>& module, bool setFocus = true);

    /**
       The actual removal of the module may be delayed until the next frame.
    */
    virtual void removeWidget(const shared_ptr<Widget>& module);

    /** Accumulated wall-clock time since init was called on this applet.
        Since this time is accumulated, it may drift from the true
        wall-clock obtained by System::time().*/
    RealTime realTime() const {
        return m_realTime;
    }

    virtual void setRealTime(RealTime r);

    /** In-simulation time since init was called on this applet.
        Takes into account simTimeSpeed.  Automatically incremented
        after ooSimulation.
    */
    SimTime simTime() const {
        return m_simTime;
    }

    virtual void setSimTime(SimTime s);

    /** For use as the \a simulationStepDuration argument of setFrameDuration */
    enum {
        /** Good for smooth animation in a high but variable-framerate system.
            Advance simulation using wall-clock time.

            Note that this will keep simulation running in "real-time" even when the system
            is running slowly (e.g., when recording video), which can lead to missed animation frames.
          */
        REAL_TIME = -100,

        /**
          Good for low frame rates when debugging, recording video, or working with a naive physics system.
          Advance simulation by the same amount as the \a wallClockTargetDuration argument
          to setFrameDuration every frame, regardless of the actual time elapsed.

          Note that when the system is running slowly, this ensures that rendering and
          simulation stay in lockstep with each other and no frames are dropped.
         */
        MATCH_REAL_TIME_TARGET = -200
    };

    /**
      \param realTimeTargetDuration Target duration between successive frames. If simulating and rendering (and all other onX methods)
      consume less time than this, then GApp will invoke onWait() to throttle.  If the frame takes more time than
      wallClockTargetDuration, then the system will proceed to the next frame as quickly as it can.

      \code
      setFrameDuration(1.0 / window()->settings().refreshRate);
      \endcode

      \param simulationStepDuration Amount to increment simulation time by each frame under normal circumstances (this is modified by setSimulationTimeScale)
         Special values are GApp::REAL_TIME, GApp::MATCH_REAL_TIME_TARGET.
    */
    virtual void setFrameDuration(RealTime realTimeTargetDuration, SimTime simulationStepDuration = MATCH_REAL_TIME_TARGET) {
        debugAssert(realTimeTargetDuration < inf());
        m_wallClockTargetDuration = (float)realTimeTargetDuration;
        m_simTimeStep = (float)simulationStepDuration;
        debugAssert(m_wallClockTargetDuration > 0.0f);
        debugAssert(m_wallClockTargetDuration < finf());
    }

    /** 1.0 / desired frame rate */
    RealTime frameDuration() const {
        return m_wallClockTargetDuration;
    }

    /** 1.0 / desired frame rate \deprecated */
    RealTime realTimeTargetDuration() const {
        return m_wallClockTargetDuration;
    }

    /** May also be REAL_TIME or MATCH_REAL_TIME_TARGET. \sa previousSimTimeStep */
    SimTime simStepDuration() const {
        return m_simTimeStep;
    }

    /** A non-negative number that is the amount that time is was advanced by in the previous frame.  Never an enum value.
        For the first frame, this is the amount that time will be advanced by if rendering runs at speed. */
    SimTime previousSimTimeStep() const {
        return m_previousSimTimeStep;
    }

    /** Actual wall-clock time elapsed between the previous two frames. \sa realTimeTargetDuration */
    RealTime previousRealTimeStep() const {
        return m_previousRealTimeStep;
    }

    /**
     Set the rate at which simulation time actually advances compared to
     the rate specified by setFrameDuration.  Set to 0 to pause simulation,
     1 for normal behavior, and use other values when fast-forwarding (greater than 1) or
     showing slow-motion (less than 1).
     */
    virtual void setSimulationTimeScale(float s) {
        m_simTimeScale = s;
    }

    float simulationTimeScale() const {
        return m_simTimeScale;
    }

    /** If true, the \a wallClockTargetDuration from setFrameDuration() is ignored
        when the OSWindow does not have focus
        and the program switches to running 4fps to avoid slowing down the foreground application.
    */
    virtual void setLowerFrameRateInBackground(bool s) {
        m_lowerFrameRateInBackground = s;
    }

    bool lowerFrameRateInBackground() const {
        return m_lowerFrameRateInBackground;
    }

protected:

    /** Change the size of the underlying Film. Called by GApp::GApp() and GApp::onEvent(). This is not an event handler.  If you want
      to be notified when your app is resized, override GApp::onEvent to handle the resize event (just don't forget to call GApp::onEvent as well).

      The guard band sizes are added to the specified width and height
      */
    virtual void resize(int w, int h);

    /**
       Load your data here.  Unlike the constructor, this catches common exceptions.
       It is called before the first frame is processed.
    */
    virtual void onInit();

    virtual void onAfterEvents();

    /**
       Unload/deallocate your data here.  Unlike the destructor, this catches common exceptions.
       It is called after the last frame is processed.
    */
    virtual void onCleanup();

    /**
       Override this with your simulation code.
       Called from GApp::run.

       The default implementation invokes
        WidgetManager::onSimulation on m_widgetManager,
        Scene::onSimulation on scene(), and
       GCamera::onSimulation on GApp::m_debugCamera in that order.

       simTime(), idealSimTime() and realTime() are incremented after
       onSimulation is called, so at the beginning of call the current
       time is the end of the previous frame.

       @param rdt Elapsed real-world time since the last call to onSimulation.

       @param sdt Elapsed sim-world time since the last call to
       onSimulation, computed by multiplying the wall-clock time by the
       simulation time rate.

       @param idt Elapsed ideal sim-world time.  Use this for perfectly
       reproducible timing results.  Ideal time always advances by the
       desiredFrameDuration * simTimeRate, no matter how much wall-clock
       time has elapsed.

       \sa onBeforeSimulation, onAfterSimulation
    */
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt);

    /** Invoked before onSimulation is run on the installed GModules and GApp.
        This is not used by most programs; it is primarily a hook for those performing
        extensive physical simulation on the GModules that need a setup and cleanup step.

        If you mutate the timestep arguments then those mutated time steps are passed
        to the onSimulation method.  However, the accumulated time will not be affected by
        the changed timestep.
    */
    virtual void onBeforeSimulation(RealTime& rdt, SimTime& sdt, SimTime& idt);

    /**
       Invoked after onSimulation is run on the installed GModules and GApp.
       Not used by most programs.
    */
    virtual void onAfterSimulation(RealTime rdt, SimTime sdt, SimTime idt);

    /**
       Rendering callback used to paint the screen.  Called automatically.
       RenderDevice::beginFrame and endFrame are called for you before this
       is invoked.

       The default implementation calls onGraphics2D and onGraphics3D.
     */
    virtual void onGraphics(RenderDevice* rd, Array<shared_ptr<Surface> >& surface,
                            Array<shared_ptr<Surface2D> >& surface2D);

    /**
       Called from the default onGraphics.
   */
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D);

    /**
       \brief Called from the default onGraphics.
       Override and implement.

       The default implementation is a full forward renderer with AO
       and post processing. See the starter sample project for equivalent
       code.

       \sa G3D::GApp::m_renderer, G3D::Renderer
   */
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface);

    /** Invoked by the default onGraphics3D to perform depth of field and motion blur
        post-processgiing on the m_framebuffer at high dynamic range. Does not include
        the tone-mapping (Film::exposeAndRender) HDR to LDR pass. */
    virtual void onPostProcessHDR3DEffects(RenderDevice* rd);

    /** Called before onGraphics.  Append any models that you want
        rendered (you can also explicitly pose and render in your
        onGraphics method).  The provided arrays will already contain
        posed models from any installed Widgets. */
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D);

    /**
       For a networked app, override this to implement your network
       message polling.
    */
    virtual void onNetwork();

    /**
       Task to be used for frame rate limiting.

       Overriding onWait is not recommended unless you have significant
       computation tasks that cannot be executed conveniently on a separate thread.

       Frame rate limiting is useful
       to avoid overloading a maching that is running background tasks and
       for situations where fixed time steps are needed for simulation and there
       is no reason to render faster.

       Default implementation System::sleep()s on waitTime (which is always non-negative)
    */
    virtual void onWait(RealTime waitTime);


    /**
       Update any state you need to here.  This is a good place for
       AI code, for example.  Called after onNetwork and onUserInput,
       before onSimulation.
    */
    virtual void onAI();


    /**
       It is recommended to override onUserInput() instead of this method.

       Override if you need to explicitly handle events raw in the order
       they appear rather than once per frame by checking the current
       system state.

       Note that the userInput contains a record of all
       keys pressed/held, mouse, and joystick state, so
       you do not have to override this method to handle
       basic input events.

       Return true if the event has been consumed (i.e., no-one else
       including GApp should process it further).

       The default implementation does nothing.

       This runs after the m_widgetManager's onEvent, so a widget may consume
       events before the App sees them.
    */
    virtual bool onEvent(const GEvent& event);

    /**
       Routine for processing user input from the previous frame.  Default implementation does nothing.
    */
    virtual void onUserInput(class UserInput* userInput);

    /**
       Invoked when a user presses enter in the in-game console.  The default implementation
       ends the program if the command is "exit".


       Sample implementation:
       \htmlonly
       <pre>
        void App::onConsoleCommand(const String& str) {
            // Add console processing here

            TextInput t(TextInput::FROM_STRING, str);
            if (t.isValid() && (t.peek().type() == Token::SYMBOL)) {
                String cmd = toLower(t.readSymbol());
                if (cmd == "exit") {
                    setExitCode(0);
                    return;
                } else if (cmd == "help") {
                    printConsoleHelp();
                    return;
                }

                // Add commands here
            }

            console->printf("Unknown command\n");
            printConsoleHelp();
        }

        void App::printConsoleHelp() {
            console->printf("exit          - Quit the program\n");
            console->printf("help          - Display this text\n\n");
            console->printf("~/ESC         - Open/Close console\n");
            console->printf("F2            - Enable first-person camera control\n");
            console->printf("F4            - Record video\n");
        }
        </pre>
        \endhtmlonly
    */
    virtual void onConsoleCommand(const String& cmd);

    /** 
        Ensures any GBuffer using \param spec has all of the fields necessary to render 
        the effects on this GApp. By default, extends the specification to handle the current
        AmbientOcclusionSettings, DepthOfFieldSettings, MotionBlurSettings, and FilmSettings.
        
        Called from GApp::onGraphics3D.

        \sa AmbientOcclusionSettings::extendGBufferSpecification
    */
    virtual void extendGBufferSpecification(GBuffer::Specification& spec);

};


/**
   Displays output on the last G3D::GApp instantiated.  If there was no GApp instantiated,
   does nothing.  Threadsafe.

   This is primarily useful for code that prints (almost) the same
   values every frame (e.g., "current position = ...") because those
   values will then appear in the same position on screen.

   For one-off print statements (e.g., "network message received")
   see G3D::consolePrintf.
 */
void screenPrintf(const char* fmt ...) G3D_CHECK_PRINTF_ARGS;

inline void screenPrintf(const String& t ...) {
    screenPrintf("%s", t.c_str());
}

}
