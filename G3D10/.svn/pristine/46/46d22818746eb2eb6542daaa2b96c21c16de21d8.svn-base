#include "App.h"

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    initGLG3D(G3DSpecification());

    GApp::Settings settings(argc, argv);
    settings.window.caption             = "Brick Breaker";
    settings.window.width               = 1280;
    settings.window.height              = 720;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;
    settings.window.defaultIconFilename = "gui/icon.png";
    settings.window.asynchronous        = false;
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(64, 64);    
    settings.hdrFramebuffer.depthGuardBandThickness = settings.hdrFramebuffer.colorGuardBandThickness.max(Vector2int16(64, 64));
    
    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = true;

    settings.dataDir                       = FileSystem::currentDirectory();

    settings.screenCapture.outputDirectory = FilePath::concat(FileSystem::currentDirectory(), "../journal");
    if (! FileSystem::exists(settings.screenCapture.outputDirectory)) {
        settings.screenCapture.outputDirectory = "";
    }
    settings.screenCapture.includeAppRevision = false;
    settings.screenCapture.includeG3DRevision = false;
    settings.screenCapture.filenamePrefix = "_";

    return App(settings).run();
}