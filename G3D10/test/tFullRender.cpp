/**
  \file test/tFullRender.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "testassert.h"
#include "App.h"


void testFullRender(bool generateGoldStandard) {
    initGLG3D();

    GApp::Settings settings;

    settings.window.caption			= "Test Renders";
    settings.window.width        = 1280; settings.window.height       = 720;
    settings.hdrFramebuffer.preferredColorFormats.clear();
    settings.hdrFramebuffer.preferredColorFormats.append(ImageFormat::RGB32F());

	// Enable vsync.  Disable for a significant performance boost if your app can't render at 60fps,
	// or if you *want* to render faster than the display.
	settings.window.asynchronous	= false;
    settings.hdrFramebuffer.depthGuardBandThickness    = Vector2int16(64, 64);
    settings.hdrFramebuffer.colorGuardBandThickness    = Vector2int16(16, 16);
    settings.dataDir				= FileSystem::currentDirectory();

    // Warning! Do not change these directories without changing the App... it relies on these directories to tell what mode we are in
    if (generateGoldStandard) { 
        settings.screenCapture.outputDirectory	= "RenderTest/GoldStandard";
    } else {
        settings.screenCapture.outputDirectory	= "RenderTest/Results";
    }  
    int result = App(settings).run();
    testAssertM(result == 0 ,"App failed to run");
}

void perfFullRender(bool generateGoldStandard) {

}
