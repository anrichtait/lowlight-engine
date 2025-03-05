/** \file App.cpp */
#include "App.h"


App::App(const GApp::Settings& settings) : super(settings) {
}


// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be automatically caught.
void App::onInit() {
    super::onInit();

    // Uncomment if you want (slightly slower) simulated binary gaze tracking
    // instead of dual-monocular gaze tracking.
    // m_gazeTracker = EmulatedGazeTracker::create(this);

    // Call setScene(shared_ptr<Scene>()) or setScene(MyScene::create()) to replace
    // the default Scene class here for extreme customization. Usually you just need to load a scene from
    // a Scene.Any file using loadScene().

    setLowerFrameRateInBackground(false);
    makeGUI();
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));
    loadScene(
        "G3D VR Portaldeck"
        //"G3D VR Holodeck"
    );
}


void App::makeGUI() {
    debugWindow->setVisible(false);
    developerWindow->videoRecordDialog->setEnabled(true);
    developerWindow->cameraControlWindow->setVisible(false);

    if (false) {
        developerWindow->profilerWindow->setVisible(true);
        Profiler::setEnabled(true);
    }

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    // Write your onGraphics3D here!
    super::onGraphics3D(rd, allSurfaces);
}


void App::onAfterLoadScene(const Any& any, const String &sceneName) {
    super::onAfterLoadScene(any, sceneName);
    setActiveCamera(debugCamera());
}


bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (super::onEvent(event)) { return true; }

    /*
    // For debugging effect levels:
    if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'i')) { 
        decreaseEffects();
        return true; 
    }
    */

    return false;
}


// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    initGLG3D(G3DSpecification());

    GApp::Settings settings(argc, argv);
    settings.vr.debugMirrorMode = 
        GApp::DebugVRMirrorMode::BOTH_EYES;
        //GApp::DebugVRMirrorMode::RIGHT_EYE_CROP;

    if (OpenVR::available()) {
        try {
            settings.vr.xrSystem = OpenVR::create();
        } catch (...) {}
    }

    if (isNull(settings.vr.xrSystem)) {
        settings.vr.xrSystem = EmulatedXR::create();
    }    

    // For testing custom AR/VR displays:
    //settings.vr.xrSystem = MonitorXR::create();

    settings.vr.disablePostEffectsIfTooSlow = false;

    settings.window.caption             = argv[0];

    // The debugging on-screen window
    settings.window.width               = 1900; settings.window.height       = 900;

    // Full screen minimizes latency (we think), but when debugging (even in release mode) 
    // it is convenient to not have the screen flicker and change focus when launching the app.
    settings.window.fullScreen          = false;
    settings.window.resizable           = false;
    settings.window.framed              = ! settings.window.fullScreen;

    // VR already provides a huge guard band
    settings.hdrFramebuffer.depthGuardBandThickness    = Vector2int16(0, 0);
    settings.hdrFramebuffer.colorGuardBandThickness    = Vector2int16(0, 0);

    // Async must be true for VR
    settings.window.asynchronous        = true;
    settings.renderer.deferredShading   = true;
    settings.renderer.orderIndependentTransparency = true;
    settings.dataDir                    = FileSystem::currentDirectory();

    const int result = App(settings).run();
    
    return result;
}