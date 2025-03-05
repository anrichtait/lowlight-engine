/**
  \file G3D-app.lib/source/VRApp.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/Log.h"
#include "G3D-base/Ray.h"
#include "G3D-gfx/glheaders.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-gfx/XR.h"
#include "G3D-app/VRApp.h"
#include "G3D-app/Draw.h"
#include "G3D-app/UserInput.h"
#include "G3D-app/MarkerEntity.h"
#include "G3D-gfx/MonitorXR.h"
#include "G3D-app/ScreenCapture.h"
#include "G3D-app/XRWidget.h"
#include "G3D-app/AmbientOcclusion.h"
#include "G3D-app/Renderer.h"

namespace G3D {

VRApp::VRApp(const GApp::Settings& settings) :
    super(makeFixedSize(settings), nullptr, nullptr, false),

    // MINIMIZE_LATENCY also happens to give good throughput in the
    // current implementation of SteamVR, although it is unclear why
    m_vrSubmitToDisplayMode(SubmitToDisplayMode::MINIMIZE_LATENCY),
    m_highQualityWarping(true),
    m_numSlowFrames(0),
    m_hudEnabled(false),
    m_hudFrame(Point3(0.0f, -0.27f, -1.2f)),
    m_hudWidth(2.0f),
    m_hudBackgroundColor(Color3::black(), 0.15f) {

    // Optimize ray casts for teleportation
    Model::setUseOptimizedIntersect(true);
    
    m_xrSystem = m_settings.vr.xrSystem;
    XR::Settings xrSettings;

    m_xrSystem->preGraphicsInit(xrSettings);
    
    // Now initialize OpenGL and RenderDevice
    initializeOpenGL(nullptr, nullptr, true, makeFixedSize(m_settings));

    m_xrSystem->postGraphicsInit(xrSettings);

    m_xrWidget = XRWidget::create(m_xrSystem, this);

    addWidget(m_xrWidget, false);

    // Detect the HMD
    m_xrSystem->updateTrackingData();

    alwaysAssertM(m_xrSystem->hmdArray().size() > 0, "No HMD found");
    const shared_ptr<XR::HMD>& hmd = m_xrSystem->hmdArray()[0];
    alwaysAssertM((unsigned int)hmd->numViews() < MAX_VIEWS, "Too many views");
    
    // Construct the eye cameras, framebuffers, and head entity
    const ImageFormat* ldrColorFormat = ImageFormat::RGB8();
    const ImageFormat* hdrColorFormat = GLCaps::firstSupportedTexture(settings.hdrFramebuffer.preferredColorFormats);
    const ImageFormat* depthFormat = GLCaps::firstSupportedTexture(settings.hdrFramebuffer.preferredDepthFormats);
    
    const float resolutionScale = 1.0f;

    for (int view = 0; view < hmd->numViews(); ++view) {
        Vector2uint32 resolution[MAX_VIEWS];
        hmd->getResolution(resolution);
        resolution[view] = Vector2uint32(iRound(float(resolution[view].x) * resolutionScale), iRound(float(resolution[view].y) * resolutionScale));
        m_hmdDeviceFramebuffer[view] = Framebuffer::create
           (Texture::createEmpty(format("G3D::VRApp::m_hmdDeviceFramebuffer[%d]/color", view), resolution[view].x, resolution[view].y, ldrColorFormat),
            Texture::createEmpty(format("G3D::VRApp::m_hmdDeviceFramebuffer[%d]/depth", view), resolution[view].x, resolution[view].y, depthFormat));
        m_hmdDeviceFramebuffer[view]->setInvertY(true);
        m_hmdDeviceFramebuffer[view]->texture(0)->visualization.documentGamma = 2.1f;

        m_hmdGBuffer[view] = GBuffer::create(m_gbufferSpecification, format("G3D::VRApp::m_hmdGBuffer[%d]", view));
        m_hmdGBuffer[view]->resize(resolution[view].x, resolution[view].y);

        // Share the depth buffer with the LDR device target
        m_hmdHDRFramebuffer[view] = Framebuffer::create
           (Texture::createEmpty(format("G3D::VRApp::m_hmdHDRFramebuffer[%d]/color", view), resolution[view].x, resolution[view].y, hdrColorFormat),
            m_hmdDeviceFramebuffer[view]->texture(Framebuffer::DEPTH));

        if (view == 0) {
            m_hmdFilm[view] = m_film;
        } else {
            m_hmdFilm[view] = Film::create();
        }

        m_ambientOcclusionArray[view] = AmbientOcclusion::create(format("G3D::VRApp::AmbientOcclusion[%d]", view));
    }

    m_gbuffer = m_hmdGBuffer[0];

    // Use slower motion to reduce discomfort
    m_debugController->setMoveRate(0.3f);

    #ifdef G3D_WINDOWS
    if (dynamic_pointer_cast<MonitorXR>(m_xrSystem)) {
        // Using two windows, so don't slow down when focus changes
        setLowerFrameRateInBackground(false);
    }
    #endif

    // If timewarp reveals the outside of the frame, make sure it is black
    renderDevice->setColorClearValue(Color3::black());
}


VRApp::~VRApp() {
    // Clear OpenGL resources before OpenGL shuts down
    const shared_ptr<XR::HMD>& hmd = m_xrSystem->hmdArray()[0];
    for (int view = 0; view < hmd->numViews(); ++view) {
        m_hmdFilm[view].reset();
        m_hmdHDRFramebuffer[view].reset();
        m_hmdDeviceFramebuffer[view].reset();
    }

    m_xrWidget = nullptr;
    m_xrSystem->cleanup();
    m_xrSystem = nullptr;
}


const GApp::Settings& VRApp::makeFixedSize(const GApp::Settings& s) {
    const_cast<GApp::Settings&>(s).window.resizable = false;
    return s;
}


void VRApp::onInit() {
    super::onInit();
    m_currentEyeIndex = 0;

    setSubmitToDisplayMode(SubmitToDisplayMode::MAXIMIZE_THROUGHPUT);

    const float freq = isNull(m_xrSystem) ? 60.0f : m_xrSystem->hmdArray()[0]->displayFrequency();
    alwaysAssertM(freq > 0.0f, "Cannot have a zero display frequency.");
    setFrameDuration(1.0f / freq);

    // Force the m_film to match the m_hmd's resolution instead of the OSWindow's
    resize(0, 0);

    m_cursorPointerTexture = Texture::fromFile(System::findDataFile("gui/cursor-pointer.png"), ImageFormat::RGBA8());

    if (notNull(developerWindow)) {
        const Vector2 developerBounds = developerWindow->bounds().wh();
        developerWindow->setRect(Rect2D::xywh(Point2(float(window()->width()), float(window()->height())) - developerBounds, developerBounds));
    }
}


void VRApp::swapBuffers() {
    // Intentionally empty...prevent subclasses from accidentally swapping buffers on their own
}


/** Returns the same CFrame, but with only yaw preserved */
CFrame VRApp::maybeRemovePitchAndRoll(const CFrame& source) const {
    if (m_settings.vr.trackingOverridesPitch && notNull(dynamic_pointer_cast<FirstPersonManipulator>(m_cameraManipulator))) {
        float x, y, z, yaw, pitch, roll;
        source.getXYZYPRRadians(x, y, z, yaw, pitch, roll);
        return CFrame::fromXYZYPRRadians(x, y, z, yaw, 0.0f, 0.0f);
    } else {
        return source;
    }
}


void VRApp::onBeforeSimulation(RealTime& rdt, SimTime& sdt, SimTime& idt) {
    GApp::onBeforeSimulation(rdt, sdt, idt);
    processTeleportation(rdt);
}


void VRApp::processTeleportation(RealTime rdt) {
    BEGIN_PROFILER_EVENT("VRApp::processTeleportation");

    // Teleportation
    const shared_ptr<Entity>& xrTrackedVolume = scene()->entity("XR Tracked Volume");
    const shared_ptr<Entity>& body = scene()->entity("XR Body");
    static bool teleportRotateBlock = false, teleportTranslateBlock = false;
    
    if (notNull(xrTrackedVolume) && notNull(body)) {
        static const float teleportTranslateDistance  =  2.0f;
        static const float teleportRotateAngleDegrees = 22.0f;
        const Array<shared_ptr<XR::Object>>& objectArray = m_xrSystem->objectArray();

        const CFrame& oldRelativeBodyFrame = xrTrackedVolume->frame().toObjectSpace(body->frame());

        // Find the largest X and Y movement of any joystick
        Vector2 maxVector;
        const JoystickIndex joystick[2] = {JoystickIndex::RIGHT, JoystickIndex::LEFT};
        for (const shared_ptr<XR::Object>& object : objectArray) {
            if (object->isController()) {
                const shared_ptr<XR::Controller>& controller = dynamic_pointer_cast<XR::Controller>(object);
                for (int j = 0; j < 2; ++j) {
                    const Vector2& v = controller->stickPosition(joystick[j]);

                    // Vive 1.0 wands have touch pads for joysticks, and they teleport too easily
                    // if we don't require a button press. Rift CV1 controllers have physical joysticks
                    // and do not need a press for unambiguous input.
                    if (controller->hasPhysicalJoystick() ||
                        controller->justPressed(GKey::CONTROLLER_LEFT_CLICK) ||
                        controller->justPressed(GKey::CONTROLLER_RIGHT_CLICK)) {
                        for (int a = 0; a < 2; ++a) {
                            if (abs(v[a]) > abs(maxVector[a])) {
                                maxVector[a] = v[a];
                            } // abs-max
                        } // axis
                    } // button press
                } // index
            } // controller
        } // object

        const shared_ptr<Entity>& head = scene()->entity("XR Head");
        const CFrame& headFrame = head->frame();
        bool teleported = false;
        if (abs(maxVector.y) < 0.5f) {
            // Release the lock, the user has backed off the control
            teleportTranslateBlock = false;
        } else if ((abs(maxVector.y) > 0.6f) && ! teleportTranslateBlock) {
            // Move in the head's view axis
            const Vector3& lookNoPitch = (headFrame.lookVector() * Vector3(1, 0, 1)).directionOrZero();

            CFrame frame = xrTrackedVolume->frame();
            frame.translation += lookNoPitch * sign(maxVector.y) * teleportTranslateDistance;
            xrTrackedVolume->setFrame(frame);
            teleportTranslateBlock = true;
            teleported = true;
        } // translation

        if (abs(maxVector.x) < 0.5f) {
            // Release the lock, the user has backed off the control
            teleportRotateBlock = false;
        } else if ((abs(maxVector.x) > 0.6f) && ! teleportRotateBlock) {
            CFrame frame = xrTrackedVolume->frame();

            // Rotate about the head
            frame = (Matrix4::yawDegrees(-sign(maxVector.x) * teleportRotateAngleDegrees) * (frame - headFrame.translation)).approxCoordinateFrame() + headFrame.translation;

            xrTrackedVolume->setFrame(frame);
            teleportRotateBlock = true;
            teleported = true;
        } // rotation


        if (m_maintainHeightOverGround) {
            // Find the ground height
            CFrame volumeFrame = xrTrackedVolume->frame();
            const float height = m_xrWidget->standingHeadHeight();
 
            // Exclude anything parented to an XR object, so that we don't step up onto them
            static Array<String> xrNames = { "XR Left Controller", "XR Right Controller", "XR Left Hand", "XR Right Hand", "XR Body", "XR Head", "XR Tracked Volume" };

            Array<String> excludeNames;
            scene()->getDescendants(xrNames, excludeNames);

            Array<shared_ptr<Entity>> excludeArray;
            scene()->getEntityArray(excludeNames, excludeArray);

            // Stored in fixed point because there is no atomic<float>
            static const float fixedPointScale = 10000.0f;
            std::atomic<int>   deltaSumFixedPoint(0);
            std::atomic<int>   numSamples(0);

            // Cast the rays

            // TODO: The threaded version crashes rendering due to some conflict with material forcing,
            // so we disabled threading and reduced the number of rays from 8 to 4, which runs in a fraction of a 
            // ms
            static const int NUM_RAYS = 4; // 8
            static const float radius = 0.10f;
//            runConcurrently(0, NUM_RAYS, [&](int i) {
            for (int i = 0; i < NUM_RAYS; ++i) {
                const float angle = pif() * 2.0f * float(i) / float(NUM_RAYS);

                // Cast a cylinder of rays from the top of the standing head, but start the ray at the elevation of the current head
                const Ray& ray = Ray::fromOriginAndDirection
                   (Point3(headFrame.translation.x + cos(angle) * radius,
                       volumeFrame.translation.y + height,
                       headFrame.translation.z + sin(angle) * radius), 
                    -Vector3::unitY(), 
                    max(0.0f, height - headFrame.translation.y + volumeFrame.translation.y + 0.1f), 
                    finf());

                float distance = finf();
                if (notNull(scene()->intersect(ray, distance, false, excludeArray))) {
                    const float delta = distance - height;
                    deltaSumFixedPoint += int(delta * fixedPointScale);
                    ++numSamples;
                } // hit
            } // for i


            if (numSamples > 0) {
                const float delta = float(deltaSumFixedPoint) / (float(numSamples) * fixedPointScale);
                static const float minVerticalTeleportThreshold = 5 * units::millimeters();

                // Don't continuously teleport on very small changes; it disrupts motion vectors
                // every time that we teleport.
                if (abs(delta) > minVerticalTeleportThreshold) {
                    volumeFrame.translation.y -= delta;
                    xrTrackedVolume->setFrame(volumeFrame);
                    teleported = true;
                }
            } // If hit ground (otherwise, there is no ground!)
        }

        if (teleported) {
            // TODO: Make "XR Body" a child of "XR Tracked Volume", so that it automatically
            // moves when we teleport?

            // Explicitly update the body frame so that it does not jump
            body->setFrame(xrTrackedVolume->frame() * oldRelativeBodyFrame);
            body->setPreviousFrame(body->frame());
            m_xrWidget->simulateBodyFrame(rdt);
        }
    } // teleportation

    END_PROFILER_EVENT();
}


void VRApp::onAfterSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onAfterSimulation(rdt, sdt, idt);
}


void VRApp::vrSubmitToDisplay() {
    const shared_ptr<XR::HMD>& hmd = m_xrSystem->hmdArray()[0];
    hmd->submitFrame(renderDevice, m_hmdDeviceFramebuffer);
}


void VRApp::setActiveCamera(const shared_ptr<Camera>& camera) {
    if (camera != m_activeCamera) {
        m_activeCamera = camera;
    }
}

void VRApp::onGraphics(RenderDevice* rd, Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    BEGIN_PROFILER_EVENT("VRApp::onGraphics");
    debugAssertM(!renderDevice->swapBuffersAutomatically(), "VRApp subclasses must not swap buffers automatically.");

    rd->pushState(); {
        debugAssert(notNull(activeCamera()));

        // Begin VR-specific
        if ((m_vrSubmitToDisplayMode == SubmitToDisplayMode::BALANCE) || (m_vrSubmitToDisplayMode == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT)) {
            // Submit the PREVIOUS frame. Do this now since there is no more work to perform for the
            // HMD image
            vrSubmitToDisplay();
        }

        // Render the main display's GUI
        if (! m_hudEnabled) {
            rd->push2D(); {
                onGraphics2D(rd, posed2D);
            } rd->pop2D();
        }

        if (notNull(screenCapture())) {
            screenCapture()->onAfterGraphics2D(rd);
        }


        if ((submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) && (! rd->swapBuffersAutomatically())) {
            // Submit the PREVIOUS frame
            super::swapBuffers();
        }        

        BEGIN_PROFILER_EVENT("RenderDevice::clear");
        rd->clear();
        END_PROFILER_EVENT();

        // Begin the NEW frame

        // No reference because we're going to mutate these
        const shared_ptr<GBuffer>     oldGBuffer = m_gbuffer;
        const shared_ptr<Framebuffer> oldFB      = m_framebuffer;
        const shared_ptr<Framebuffer> oldDevice  = m_deviceFramebuffer;
        const shared_ptr<Camera>      oldCamera  = activeCamera();

        for (m_currentEyeIndex = 0; m_currentEyeIndex < 2; ++m_currentEyeIndex) {
            m_gaze = &m_gazeArray[m_currentEyeIndex];
            // Switch to eye render target for the display itself (we assume that onGraphics3D will probably
            // bind its own HDR buffer and then resolve to this one.)
            m_deviceFramebuffer = m_hmdDeviceFramebuffer[m_currentEyeIndex];
            m_film = m_hmdFilm[m_currentEyeIndex];
            m_ambientOcclusion = m_ambientOcclusionArray[m_currentEyeIndex];


            if (notNull(scene())) {
                scene()->lightingEnvironment().ambientOcclusion = m_ambientOcclusion;
            }

            // Swap out the underlying framebuffer that is "current" on the GApp
            m_framebuffer = m_hmdHDRFramebuffer[m_currentEyeIndex];
            rd->pushState(m_deviceFramebuffer); {

                // Parameters were copied from the body camera
                setActiveCamera(m_xrWidget->eyeCamera(m_currentEyeIndex));
                if (m_forceDiskFramebuffer) {
                    activeCamera()->filmSettings().setDiskFramebuffer(true);
                    activeCamera()->depthOfFieldSettings().setDiskFramebuffer(true);
                }

                m_gbuffer = m_hmdGBuffer[m_currentEyeIndex];

                onGraphics3D(rd, posed3D);
                debugAssertGLOk();
            } rd->popState();
        }

        setActiveCamera(oldCamera);

        m_gbuffer = oldGBuffer;
        m_framebuffer = oldFB;
        m_deviceFramebuffer = oldDevice;

# if 0 // TODO: HUD
        // Increment to use next texture, just before writing
        if (m_hudEnabled) {
            m_hmd->hudFramebufferQueue->advance();
            rd->push2D(m_hmd->hudFramebufferQueue->currentFramebuffer()); {
                rd->setColorClearValue(m_hudBackgroundColor);
                rd->clear();
                onGraphics2D(rd, posed2D);

                // Draw the cursor, if visible
                if (window()->mouseHideCount() < 1) {
                    const Point2 cursorPointerTextureHotspot(1, 1);
                    rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
                    // Clamp to screen so that the mouse is never invisible
                    const Point2 mousePos = clamp(userInput->mouseXY(), Point2(0.0f, 0.0f), Point2(float(window()->width()), float(window()->height())));
                    Draw::rect2D(m_cursorPointerTexture->rect2DBounds() + mousePos - cursorPointerTextureHotspot, rd, Color3::white(), m_cursorPointerTexture);
                }
            } rd->pop2D();
        }

#endif
        
        if (m_vrSubmitToDisplayMode == SubmitToDisplayMode::MINIMIZE_LATENCY) {
            // Submit the CURRENT frame
            vrSubmitToDisplay();
        }

        if (m_settings.vr.debugMirrorMode != DebugVRMirrorMode::NONE) {
            debugAssertGLOk();
            // Mirror to the screen
            rd->push2D(m_osWindowDeviceFramebuffer); {
                rd->setColorClearValue(Color3::black());
                rd->clear();
                rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);
                if (m_settings.vr.debugMirrorMode == DebugVRMirrorMode::BOTH_EYES) {
                    for (int eye = 0; eye < 2; ++eye) {
                        const shared_ptr<Texture>& finalImage = m_hmdDeviceFramebuffer[eye]->texture(0);

                        // Find the scale needed to fit both images on screen
                        const float scale = min(float(rd->width()) * 0.5f / float(finalImage->width()), float(rd->height()) / float(finalImage->height()));
                        const int xShiftDirection = 2 * eye - 1;

                        const float width = finalImage->width()  * scale;
                        const float height = finalImage->height() * scale;

                        const Rect2D& rect = Rect2D::xywh((rd->width() + width * float(xShiftDirection - 1)) * 0.5f, 0.0f, width, height);
                        Draw::rect2D(rect, rd, Color3::white(), finalImage, Sampler::video(), true, finalImage->visualization.documentGamma / 2.1f);
                    }
                } else {
                    const int eye = 1;
                    const shared_ptr<Texture>& finalImage = m_hmdDeviceFramebuffer[eye]->texture(0);

                    Rect2D rect;
                    if (m_settings.vr.debugMirrorMode == DebugVRMirrorMode::RIGHT_EYE_FULL) {
                        rect = rd->viewport().largestCenteredSubRect(float(finalImage->width()), float(finalImage->height()));
                    } else {
                        // Cropped
                        // Find the scale needed to fit both images on screen
                        const float scale = max(float(rd->width()) / float(finalImage->width()), float(rd->height()) / float(finalImage->height()));
                        const float width = finalImage->width() * scale;
                        const float height = finalImage->height() * scale;
                        rect = Rect2D::xywh((rd->width() - width) / 2.0f, (rd->height() - height) / 2.0f, width, height);
                    }
                    Draw::rect2D(rect, rd, Color3::white(), finalImage, Sampler::video(), true, finalImage->visualization.documentGamma / 2.1f);
                }
            } rd->pop2D();

            if (notNull(screenCapture())) {
                screenCapture()->onAfterGraphics3D(rd);
            }
        }

    } rd->popState();

    maybeAdjustEffects();
    END_PROFILER_EVENT();
}


void VRApp::maybeAdjustEffects() {
    const RealTime frameTime = 1.0 / renderDevice->stats().frameRate;
    const RealTime targetTime = realTimeTargetDuration();

    // Allow 10% overhead for roundoff, and allow to drop to 
    // 1/2 the desired frame rate if using timewarp
    if (m_settings.vr.disablePostEffectsIfTooSlow && (frameTime > targetTime * 0.60f)) {
        ++m_numSlowFrames;
        if (m_numSlowFrames > MAX_SLOW_FRAMES) {
            m_numSlowFrames = 0;
            decreaseEffects();
        }
    } // Over time
}


void VRApp::decreaseEffects() {
    // Disable some effects
    if (scene()->lightingEnvironment().ambientOcclusionSettings.enabled && scene()->lightingEnvironment().ambientOcclusionSettings.useDepthPeelBuffer) {
        // Use faster AO
        scene()->lightingEnvironment().ambientOcclusionSettings.useDepthPeelBuffer = false;
        debugPrintf("VRApp::decreaseEffects() Disabled depth-peeled AO to increase performance.\n");

    } else if (activeCamera()->filmSettings().bloomStrength() > 0) {
        // Turn off bloom
        activeCamera()->filmSettings().setBloomStrength(0.0f);
        debugPrintf("VRApp::decreaseEffects() Disabled bloom to increase performance.\n");

    } else if (scene()->lightingEnvironment().ambientOcclusionSettings.enabled) {

        if (scene()->lightingEnvironment().ambientOcclusionSettings.numSamples > 3) {
            // Decrease AO
            scene()->lightingEnvironment().ambientOcclusionSettings.numSamples = max(2, (int)(scene()->lightingEnvironment().ambientOcclusionSettings.numSamples * 0.8f - 1));
            debugPrintf("VRApp::decreaseEffects() Decreased AO sample taps to increase performance.\n");
        } else {
            // Disable AO
            scene()->lightingEnvironment().ambientOcclusionSettings.enabled = false;
            debugPrintf("VRApp::decreaseEffects() Disabled AO to increase performance.\n");
        }

    } else if (activeCamera()->filmSettings().antialiasingHighQuality()) {
        // Disable high-quality FXAA
        activeCamera()->filmSettings().setAntialiasingHighQuality(false);
        debugPrintf("VRApp::decreaseEffects() Disabled high-quality antialiasing to increase performance.\n");

    } else if (activeCamera()->filmSettings().antialiasingEnabled()) {
        // Disable FXAA
        activeCamera()->filmSettings().setAntialiasingEnabled(false);
        debugPrintf("VRApp::decreaseEffects() Disabled antialiasing to increase performance.\n");
    } else if (activeCamera()->filmSettings().temporalAntialiasingEnabled()) {
        // Disable TAA
        activeCamera()->filmSettings().setTemporalAntialiasingEnabled(false);
        debugPrintf("VRApp::decreaseEffects() Disabled temporal to increase performance.\n");
    }
}


void VRApp::onCleanup() {
    // Called after the application loop ends.  Place a majority of cleanup code
    // here instead of in the destructor so that exceptions can be caught. 
    GApp::onCleanup();
}


void VRApp::onAfterLoadScene(const Any& any, const String& sceneName) {
    super::onAfterLoadScene(any, sceneName);

    m_debugCamera->copyParametersFrom(scene()->defaultCamera());
    m_debugCamera->setTrack(nullptr);
    m_debugController->setFrame(m_debugCamera->frame());

    setActiveCamera(scene()->defaultCamera());
    m_activeCameraMarker->setTrack(Entity::EntityTrack::create(&*m_activeCameraMarker, "XR Head", &(*scene())));

    // Give a grace period for initialization
    m_numSlowFrames = -240;

    // Default to good warping
    m_highQualityWarping = true;
    
    // Drop the default camera to the ground and use that as the initial VR room position
    float groundDistance = finf();
    CFrame initialPlayerFootFrame;
    if (notNull(scene()->intersect(Ray::fromOriginAndDirection(activeCamera()->frame().translation, -Vector3::unitY()), groundDistance))) {
        initialPlayerFootFrame = (activeCamera()->frame() - Vector3(0, groundDistance, 0));
    } else {
        // No ground, assume that the camera is Morgan's head height
        const float scenePlayerHeight = 1.68f;
        initialPlayerFootFrame = (activeCamera()->frame() - Vector3(0, scenePlayerHeight, 0));
    }

    // Keep the yaw and discard the rest of the original rotation
    float x, y, z, yaw, pitch, roll;
    initialPlayerFootFrame.getXYZYPRRadians(x, y, z, yaw, pitch, roll);
    initialPlayerFootFrame = CFrame::fromXYZYPRRadians(x, y, z, yaw, 0.0f, 0.0f);
    initialPlayerFootFrame = VRApp::maybeRemovePitchAndRoll(initialPlayerFootFrame);

    // Allow XRWidget to add its elements to the scene
    m_xrWidget->onSimulation(0, 0, 0);

    const shared_ptr<Entity>& head = scene()->entity("XR Head");
    const shared_ptr<Entity>& trackedVolume = scene()->entity("XR Tracked Volume");
    alwaysAssertM(notNull(trackedVolume), "Can't find XR Tracked Volume");
    alwaysAssertM(notNull(head), "Can't find XR Head");

    // We want to put the player at initialPlayerFootFrame, but all we can move
    // is the relative coordinate transformation of the room.
    CFrame headOffset = VRApp::maybeRemovePitchAndRoll(trackedVolume->frame().toObjectSpace(head->frame()));
    // Ignore vertical offset, pitch, and roll
    headOffset.translation.y = 0;

    const CFrame& initialTrackedVolumeFrame = initialPlayerFootFrame * headOffset.inverse();
    // Teleport the room
    trackedVolume->setFrame(initialTrackedVolumeFrame);

    // Change to VR cutouts for all full-screen effects.
    // The AO cutout can be more aggressive than other effects because it has extreme
    // temporal filtering, so the region exposed under timewarp will be simulated by 
    // our own reprojection blur in the event of motion.
    if (m_forceDiskFramebuffer) {
        scene()->lightingEnvironment().ambientOcclusionSettings.diskFramebuffer = true;
        m_renderer->setDiskFramebuffer(true);
    }

    addAvatar();
}


void VRApp::addAvatar() {
    const Scene::VRSettings::Avatar& avatar = scene()->vrSettings().avatar;
    
    if (avatar.addHandEntity) {
        scene()->createModel(Any("model/vr/leftHand.ArticulatedModel.Any"), "VRApp Left Hand Model");
        scene()->createModel(Any("model/vr/rightHand.ArticulatedModel.Any"), "VRApp Right Hand Model");

        scene()->createEntity("leftHand", Any::parse(STR(VisibleEntity {
            model = "VRApp Left Hand Model"; 
            track = entity("XR Left Hand"); 
            shouldBeSaved = false;
            })));

        scene()->createEntity("rightHand", Any::parse(STR(VisibleEntity {
            model = "VRApp Right Hand Model"; 
            track = entity("XR Right Hand"); 
            shouldBeSaved = false;
            }))); 
    }

    if (avatar.addTorsoEntity) {
        scene()->createModel(Any("model/vr/torso.ArticulatedModel.Any"), "VRApp Torso Model");
        scene()->createEntity("torso", Any::parse(STR(VisibleEntity {
                model = "VRApp Torso Model"; 
                track = transform(entity("XR Body"), Point3(0, -0.15f, 0.05f));
                shouldBeSaved = false;
            }))); 
    }
}


bool VRApp::onEvent(const GEvent& event) {
    // Handle super-class events
    if (super::onEvent(event)) { return true; }

    // HUD toggle
    if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) { 
        m_hudEnabled = ! m_hudEnabled;
        if (m_hudEnabled) {
            // Capture the mouse to the window
            window()->incInputCaptureCount();
        } else {
            window()->decInputCaptureCount();
        }
        return true; 
    } else if ((event.type == GEventType::MOUSE_MOTION) && m_hudEnabled) {
        // If the mouse moved outside of the allowed bounds, move it back
        const Point2& p = event.mousePosition();
        const Point2& size = m_hmdDeviceFramebuffer[0]->vector2Bounds() - Vector2::one(); 
        if ((p.x < 0) || (p.y < 0) || (p.x > size.x) || (p.y > size.y)) {
            window()->setRelativeMousePosition(p.clamp(Vector2::zero(), size));
        }
        return false;
    }

    return false;
}


} // namespace
