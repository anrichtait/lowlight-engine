/**
  \file tools/viewer/main.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "App.h"

#if defined(G3D_VER) && (G3D_VER < 100000)
#   error Requires G3D 10
#endif

G3D_START_AT_MAIN();

App* app;

int main(int argc, char** argv) {
    // Create the log file in the directory of the executable, not the data file
    G3DSpecification spec;
    const String& s = FilePath::concat(FilePath::parent(System::currentProgramFilename()), "log.txt");
    spec.logFilename = s.c_str();
	initGLG3D(spec);

    String filename;
    
    if (argc > 1) {
        filename = argv[1];
    }

    // Force the log to start and write out information before we hit the first
    // System::findDataFile call
    logLazyPrintf("Launch command: %s %s\n", argv[0], filename.c_str());
    char b[2048];
    (void)getcwd(b, 2048);
    logPrintf("cwd = %s\n\n", b);
    
    GApp::Settings settings;

    settings.writeLicenseFile = false;
    settings.window.resizable = true;
    
#   ifdef G3D_OSX
        settings.window.defaultIconFilename = System::findDataFile("icon/G3D-128.png", false);
#   else
        settings.window.defaultIconFilename = System::findDataFile("icon/G3D-64.png", false);
#   endif

    settings.window.width     = 1596;
    settings.window.height    = 720;
    settings.window.caption   = "G3D Asset Viewer";
    
    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = true;

    logLazyPrintf("---------------------------------------------------------------------\n\n");
    logPrintf("Invoking App constructor\n");

    try {
        app = new App(settings, filename);
        const int r = app->run();
        delete app;
        return r;
    } catch (const FileNotFound& e) {
        logPrintf("Uncaught exception at main(): %s\n", e.message.c_str());
        alwaysAssertM(false, e.message);
    } catch (const G3D::String& e) {
        logPrintf("Uncaught exception at main(): %s\n", e.c_str());
        alwaysAssertM(false, e);
    } catch (...) {
        logPrintf("Uncaught exception at main().\n");
    }
}
