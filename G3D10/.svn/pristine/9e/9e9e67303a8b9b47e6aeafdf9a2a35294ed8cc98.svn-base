/**
  \file tools/viewer/App.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2023, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "App.h"
#include "ArticulatedViewer.h"
#include "PointViewer.h"
#include "TextureViewer.h"
#include "FontViewer.h"
#include "MD2Viewer.h"
#include "MD3Viewer.h"
#include "GUIViewer.h"
#include "EmptyViewer.h"
#include "VideoViewer.h"
#include "IconSetViewer.h"
#include "EventViewer.h"
#include "SceneViewer.h"

App::App(const GApp::Settings& settings, const G3D::String& file) :
    GApp(settings),
    viewer(nullptr),
    filename(file) {

    logPrintf("App()\n");
    m_debugTextColor = Color3::black();
    m_debugTextOutlineColor = Color3::white();
    activeCamera()->filmSettings().setVignetteBottomStrength(0);
    activeCamera()->filmSettings().setVignetteTopStrength(0);
    activeCamera()->filmSettings().setVignetteSizeFraction(0);
    catchCommonExceptions = true;
}


void App::onInit() {
    logPrintf("App::onInit()\n");
    GApp::onInit();

	renderDevice->setSwapBuffersAutomatically(true);
    showRenderingStats = false;

#ifndef GLEW_EGL
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->setVisible(false);
    developerWindow->videoRecordDialog->setCaptureGui(false);
    developerWindow->moveTo(Point2(window()->width() - developerWindow->rect().width(), window()->height() - developerWindow->rect().height()));
    developerWindow->sceneEditorWindow->setVisible(false);
#endif

    activeCamera()->filmSettings().setBloomStrength(0.15f);
    activeCamera()->filmSettings().setBloomRadiusFraction(0.05f);
    activeCamera()->filmSettings().setAntialiasingEnabled(true);
    activeCamera()->filmSettings().setAntialiasingHighQuality(true);
    activeCamera()->filmSettings().setCelluloidToneCurve();

    if (! filename.empty()) {
        window()->setCaption(filenameBaseExt(filename) + " - G3D Viewer");
    }

    lighting = std::make_shared<LightingEnvironment>();
    lighting->lightArray.clear();
    // The spot light is designed to just barely fit the 3D models.  Note that it has no attenuation
    lighting->lightArray.append(Light::spotTarget("Key", Point3(-45, 125, 65), Point3::zero(), 10 * units::degrees(), Power3(17.0f, 16.75f, 16.2f), 1, 0, 0, true, 8192));
    lighting->lightArray.last()->shadowMap()->setBias(0.05f);

    lighting->lightArray.append(Light::spotTarget("Fill", Point3(160, -200, 160), Point3::zero(), 10 * units::degrees(), Power3(1.5f, 2.0f, 2.5f), 1, 0, 0, false, 8192));
    lighting->lightArray.append(Light::spotTarget("Back", Point3(20, 30, -160), Point3::zero(), 10 * units::degrees(), Power3(1.5f, 1.20f, 1.20f), 1, 0, 0, false, 8192));


    Texture::Encoding e;
    e.readMultiplyFirst = Color4(Color3(0.5f));
    e.format = ImageFormat::R11G11B10F();
    Texture::Preprocess p;
    p.gammaAdjust = 2.8f; // Reduce the highs in the evt map
    lighting->environmentMapArray.append(Texture::fromFile(System::findDataFile("uffizi/uffizi-*.exr"), e, Texture::DIM_CUBE_MAP));

    lighting->ambientOcclusionSettings.numSamples   = 24;
    lighting->ambientOcclusionSettings.radius       = 0.75f * units::meters();
    lighting->ambientOcclusionSettings.intensity    = 2.0f;
    lighting->ambientOcclusionSettings.bias         = 0.06f * units::meters();
    lighting->ambientOcclusionSettings.blurStepSize = 1;
    lighting->ambientOcclusionSettings.useDepthPeelBuffer = true;
    lighting->ambientOcclusionSettings.highQualityBlur = true;
    lighting->ambientOcclusionSettings.useNormalsInBlur = true;
    lighting->ambientOcclusionSettings.temporallyVarySamples = true;


    activeCamera()->setFarPlaneZ(-finf());
    activeCamera()->setNearPlaneZ(-0.05f);
    activeCamera()->filmSettings().setTemporalAntialiasingEnabled(false);

    // Don't clip to the near plane
    glDisable(GL_DEPTH_CLAMP);	
    colorClear = Color3::white() * 0.9f;

    // modelController = ThirdPersonManipulator::create();
    m_gbufferSpecification.encoding[GBuffer::Field::CS_POSITION_CHANGE].format = nullptr;
    m_gbufferSpecification.encoding[GBuffer::Field::SS_POSITION_CHANGE].format = ImageFormat::RG16F();
    // For debugging texture coords
    m_gbufferSpecification.encoding[GBuffer::Field::TEXCOORD0].format = ImageFormat::RG16F();
    gbuffer()->setSpecification(m_gbufferSpecification);

    // Force allocation
    gbuffer()->resize(256, 256);
    gbuffer()->prepare(renderDevice, activeCamera(), 0, 0, settings().hdrFramebuffer.depthGuardBandThickness, settings().hdrFramebuffer.colorGuardBandThickness);

    setViewer(filename);

# ifndef GLEW_EGL
    GuiPane* pane = debugPane->addPane("", GuiTheme::NO_PANE_STYLE);
    pane->addCheckBox("Show Instructions", &showInstructions);
    pane->pack();
    GuiTextureBox* t = nullptr;
    GuiControl* prev = nullptr;
    const Vector2 s(256, 144);
    const float z = 0.2f;
    t = new GuiTextureBox(debugPane, "CS Normal", this, m_gbuffer->texture(GBuffer::Field::CS_NORMAL));
    t->setSizeFromInterior(s); t->setViewZoom(z); debugPane->addCustom(t); prev = t;

    t = new GuiTextureBox(debugPane, "TexCoord0", this, m_gbuffer->texture(GBuffer::Field::TEXCOORD0));
    t->setSizeFromInterior(s); t->setViewZoom(z); debugPane->addCustom(t); t->moveRightOf(prev); prev = t;

    t = new GuiTextureBox(debugPane, "Lambertian", this, m_gbuffer->texture(GBuffer::Field::LAMBERTIAN));
    t->setSizeFromInterior(s); t->setViewZoom(z); debugPane->addCustom(t); t->moveRightOf(prev); prev = t;

    t = new GuiTextureBox(debugPane, "Glossy", this, m_gbuffer->texture(GBuffer::Field::GLOSSY));
    t->setSizeFromInterior(s); t->setViewZoom(z); debugPane->addCustom(t); t->moveRightOf(prev); prev = t;

    t = new GuiTextureBox(debugPane, "Smoothness", this, m_gbuffer->texture(GBuffer::Field::GLOSSY));
    t->setSettings(Texture::Visualization::AasL);
    t->setSizeFromInterior(s); t->setViewZoom(z); debugPane->addCustom(t); t->moveRightOf(prev); prev = t;

    t = new GuiTextureBox(debugPane, "Emissive", this, m_gbuffer->texture(GBuffer::Field::EMISSIVE));
    t->setSizeFromInterior(s); t->setViewZoom(z); debugPane->addCustom(t); t->moveRightOf(prev); prev = t;
    //debugPane->setHeight(300);
    debugPane->pack();
    debugWindow->pack();
# endif

    setSubmitToDisplayMode(SubmitToDisplayMode::EXPLICIT);

    logPrintf("Done App::onInit()\n");
}


void App::onCleanup() {
    delete viewer;
    viewer = nullptr;
}


bool App::onEvent(const GEvent& e) {
    if (notNull(viewer)) {
        if (viewer->onEvent(e, this)) {
            // Viewer consumed the event
            return true;
        }
    }

    switch (e.type) {
    case GEventType::FILE_DROP:
        {
            Array<G3D::String> fileArray;
            window()->getDroppedFilenames(fileArray);
            setViewer(fileArray[0]);
            return true;
        }

    case GEventType::KEY_DOWN:
        if (e.key.keysym.sym == GKey::F5) { 
            Shader::reloadAll();
            return true;
        } else if (e.key.keysym.sym == GKey::F3) {
            showDebugText = ! showDebugText;
            return true;
        } else if (e.key.keysym.sym == GKey::F8) {
            Array<shared_ptr<Texture> > output;
            renderCubeMap(renderDevice, output, activeCamera(), shared_ptr<Texture>(), 2048);

            const CubeMapConvention::CubeMapInfo& cubeMapInfo = Texture::cubeMapInfo(CubeMapConvention::DIRECTX);
            for (int f = 0; f < 6; ++f) {
                const CubeMapConvention::CubeMapInfo::Face& faceInfo = cubeMapInfo.face[f];

                const shared_ptr<Image>& temp = Image::fromPixelTransferBuffer(output[f]->toPixelTransferBuffer(ImageFormat::RGB8()));
                temp->flipVertical();
                temp->rotateCW(toRadians(90.0f) * -faceInfo.rotations);
                if (faceInfo.flipY) { temp->flipVertical();   }
                if (faceInfo.flipX) { temp->flipHorizontal(); }
                temp->save(format("cube-%s.png", faceInfo.suffix.c_str()));
            }
            return true;
        } else if (e.key.keysym.sym == 'v') {
            // Event viewer
            if (isNull(dynamic_cast<EventViewer*>(viewer))) {
                setViewer("<events>");
                return true;
            }
        }
        break;
        
    default:;
    }

    // Must call after processing events to prevent default .ArticulatedModel.Any file-drop functionality
    if (GApp::onEvent(e)) {
        return true;
    }
    
    return false;    
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    // Make the camera spin when the debug controller is not active
    if (false) {
        static float angle = 0.0f;
        angle += (float)rdt;
        const float radius = 5.5f;
        activeCamera()->setPosition(Vector3(cos(angle), 0, sin(angle)) * radius);
        activeCamera()->lookAt(Vector3(0,0,0));
    }

    // let viewer sim with time step if needed
    if (notNull(viewer)) {
        viewer->onSimulation(rdt, sdt, idt);
    }

#ifndef GLEW_EGL
    developerWindow->moveTo(Point2(window()->width() - developerWindow->rect().width(), window()->height() - developerWindow->rect().height()));

    static bool firstTime = true;
    if (developerWindow->visible() && firstTime) {
        // Switch to pro mode
        debugWindow->setVisible(true);
        showInstructions = false;
        firstTime = false;
    }
#endif
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    // Render the file that is currently being viewed
    if (notNull(viewer)) {
        GApp::onGraphics3D(rd, allSurfaces);
        viewer->onGraphics3D(rd, allSurfaces);
    }
}


void App::onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    GApp::onPose(posed3D, posed2D);

    // Append any models to the arrays that you want to later be rendered by onGraphics()
    if (notNull(viewer)) {
        viewer->onPose(posed3D, posed2D);
    }
}


void App::onGraphics2D(RenderDevice *rd, Array< shared_ptr<Surface2D> > &surface2D) {
    rd->setColorClearValue(Color3::white());
    viewer->onGraphics2D(rd, surface2D);
    GApp::onGraphics2D(rd, surface2D);
}


/** Returns the first file found with any of the extensions provided */
static String findWithExt(const Array<String>& filenames, const Array<String>& extensions) {
    for (const String& filename : filenames) {
        for (const String& ext : extensions) {
            if (toLower(filenameExt(filename)) == toLower(ext)) {
                return filename;
            }
            // Special case for handling *.[x].Any files
            else if (toLower(filenameExt(filename)) == "any" && endsWith(toLower(filename), toLower(ext))) {
                return filename;
            }
        }
    }
    return "";
}

void App::setViewer(const G3D::String& newFilename) {
    logPrintf("App::setViewer(\"%s\")\n", filename.c_str());
# ifndef GLEW_EGL
    drawMessage("Loading " + newFilename);
# endif
    filename = newFilename;

    activeCamera()->setFrame(CFrame::fromXYZYPRDegrees(-11.8f,  25.2f,  31.8f, -23.5f, -39.0f,   0.0f));
    m_debugController->setFrame(activeCamera()->frame());

    delete viewer;
    viewer = nullptr;

    /* Reset settings that may have been overwritten by previous viewer */
    scene()->clear();
    setActiveCamera(m_debugCamera);
    activeCamera()->filmSettings().setBloomStrength(0.15f);
    activeCamera()->filmSettings().setBloomRadiusFraction(0.05f);
    activeCamera()->filmSettings().setAntialiasingEnabled(true);
    activeCamera()->filmSettings().setAntialiasingHighQuality(true);
    activeCamera()->filmSettings().setCelluloidToneCurve();
    activeCamera()->filmSettings().setVignetteBottomStrength(0);
    activeCamera()->filmSettings().setVignetteTopStrength(0);
    activeCamera()->filmSettings().setVignetteSizeFraction(0);
    activeCamera()->filmSettings().setTemporalAntialiasingEnabled(false);
#ifndef GLEW_EGL
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
#endif

    showDebugText = true;

    if (filename == "<events>") {
        viewer = new EventViewer();
    } else {
        String ext  = toLower(filenameExt(filename));
        String base = toLower(filenameBase(filename));
        
        // Models extensions not explicitly prioritized (all lower case!)
        const Array<String> otherModelExts = { "3ds", "ifs", "ply2", "off", "stl", "lwo", "gltf", "stla", "dae", "glb"};

        // Handle zipped files here (extract and find "priority file" from within)
        if (ext == "zip") {
            // List the files within the zip
            Array<String> filenames;
            FileSystem::list(filename + "\\*", filenames);
        
            // Look for included filenames w/ priority
            String toLoad = findWithExt(filenames, { ".ArticulatedModel.Any" });            // Articulated models
            if (toLoad.empty()) // Figure out BSP here... }                                 // BSP
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "obj" }); }             // OBJ
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "fbx" }); }             // FBX
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "ply" }); }             // PLY
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "rbxl" }); }            // RBXL
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "rbxm" }); }            // RBXM
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "rbxl" }); }            // RBXL
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "rbxm" }); }            // RBXM
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "rbxlx" }); }
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "rbxmx" }); }
            if (toLoad.empty()) { toLoad = findWithExt(filenames, otherModelExts); }        // All other model formats

            // Handle images
            if (toLoad.empty()) {                                                           // All images
                for (const String& fname : filenames) {
                    if (Texture::isSupportedImage(fname)) { toLoad = fname;  }
                }
            }
            
            if (toLoad.empty()) { toLoad = findWithExt(filenames, { "fnt" }); }             // Font files

            if (!toLoad.empty()) {
                // Setup the parsing here
                filename = toLoad;
                ext = toLower(filenameExt(filename));
                base = toLower(filenameBase(filename));

                drawMessage("Loading " + filename);                     // Update the draw message w/ the "sub-file"
            }
        }

        // Sets the viewer based on filetype. Viewer approximately eq scene.
        if ((ext == "obj") ||
            (ext == "fbx") ||
            (ext == "ply")  ||
            (ext == "rbxl") ||
            (ext == "rbxm") ||
            (ext == "rbxmx") ||
            (ext == "bsp") ||
            (otherModelExts.contains(ext)) ||
            (ext == "any" && endsWith(base, ".universalmaterial")) ||
            (ext == "any" && endsWith(base, ".articulatedmodel"))) {

            showDebugText = false;
            viewer = new ArticulatedViewer();
            lighting->ambientOcclusion = m_ambientOcclusion;
            scene()->setLightingEnvironment(lighting);

        } else if ((ext == "rbxl") ||
                   (ext == "rbxlx") ||
                   (ext == "any" && endsWith(base, ".scene"))) {

            viewer = new SceneViewer();

        } else if ((ext == "points")  ||
                   (ext == "xyz") ||
                   (ext == "vox")) {

            viewer = new PointViewer();

        } else if (Texture::isSupportedImage(filename)) {
            
            // Images can be either a Texture or a Sky, TextureViewer will figure it out
            viewer = new TextureViewer();
            
            // Angle the camera slightly so a sky/cube map doesn't see only 1 face
            m_debugController->setFrame(Matrix3::fromAxisAngle(Vector3::unitY(), (float)halfPi() / 2.0f) * Matrix3::fromAxisAngle(Vector3::unitX(), (float)halfPi() / 2.0f));
            
        } else if (ext == "fnt") {
            
            viewer = new FontViewer(debugFont);
            
/*        } else if (ext == "bsp") {
            
            viewer = new BSPViewer();
  */          
        } else if (ext == "md2") {
            activeCamera()->setFrame(CFrame::fromXYZYPRDegrees(0, 0, 3, 0, 0, 0));
            m_debugController->setFrame(activeCamera()->frame());
            viewer = new MD2Viewer();
            lighting->ambientOcclusion = m_ambientOcclusion;
            scene()->setLightingEnvironment(lighting);
            
        } else if (ext == "md3") {

            viewer = new MD3Viewer();
            lighting->ambientOcclusion = m_ambientOcclusion;
            scene()->setLightingEnvironment(lighting);

        } else if (ext == "gtm") {
            
            viewer = new GUIViewer(this);
            
        } else if (ext == "icn") {
            
            viewer = new IconSetViewer(debugFont);
            
        } else if (ext == "pk3") {
            // Something in Quake format - figure out what we should load
            Array <String> files;
            bool set = false;
            
            // First, try for a .bsp map
            G3D::String search = filename + "/maps/*";
            FileSystem::getFiles(search, files, true);
            
            for (int t = 0; t < files.length(); ++t) {
                
                if (filenameExt(files[t]) == "bsp") {
                    
                    filename = files[t];
                    viewer = new ArticulatedViewer();
                    set = true;
                }
            }
            if (! set) {
                viewer = new EmptyViewer();
            }
            
        } else if (ext == "avi" || ext == "wmv" || ext == "mp4" || ext == "asf" || 
                   (ext == "mov") || (ext == "dv") || (ext == "qt") || (ext == "asf") ||
                   (ext == "mpg")) {
            viewer = new VideoViewer();
            
        } /*else if (ext == "usd" || ext == "usda" || ext == "usdc") {
            viewer = new SceneViewer();

        } */else {
            
            viewer = new EmptyViewer();
            
        }
    }
    // Sets up viewer / renders model 
    if (notNull(viewer)) {
        viewer->onInit(filename, this);
    }
    
    if (filename != "") {
        if (filename == "<events>") {
            window()->setCaption("Events - G3D Viewer");
        } else {
            window()->setCaption(filenameBaseExt(filename) + " - G3D Viewer");
        } 
    }
    
    logPrintf("Done App::setViewer(...)\n");
}
