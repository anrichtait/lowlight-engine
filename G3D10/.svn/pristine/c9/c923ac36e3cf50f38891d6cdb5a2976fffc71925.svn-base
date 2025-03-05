/** \file App.cpp */
#include "App.h"

G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);

    settings.window.caption             = argv[0];
    settings.window.width               = 1280; settings.window.height       = 720;
    settings.screenCapture.includeAppRevision = false;
    settings.screenCapture.includeG3DRevision = false;
    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(256, 256);
    settings.hdrFramebuffer.colorGuardBandThickness = settings.hdrFramebuffer.depthGuardBandThickness;
    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = true;

    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {}


void App::onInit() {
    GApp::onInit();

    m_pathTracer = PathTracer::create(
        //OptiXTriTree::create());
        EmbreeTriTree::create());
    //m_options.raysPerPixel = 16;
    //m_options.maxScatteringEvents = 1;

    window()->setCaption(String("G3D Path Tracer (using ") + m_pathTracer->triTree()->className() + ")");
    makeGUI();
    loadScene(
#ifdef G3D_DEBUG
        "G3D Simple Cornell Box (Area Light)"
#else
        //"G3D Sponza (White)"
        //"G3D Triangle"
        "G3D Simple Cornell Box (Area Light)"
        //"G3D Simple Cornell Box (Glossy)"
        //"G3D Simple Cornell Box (Water)"
        //"G3D Simple Cornell Box (Mirror)"
        //"G3D Simple Cornell Box (Spheres)"
        //"G3D Sports Car"
        // "G3D Debug Area Light"
        // "G3D Debug Triangle"
#endif
    );
}


void App::makeGUI() {
    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);
    developerWindow->cameraControlWindow->setVisible(false);
    showRenderingStats = false;

    GuiTabPane* tabPane = debugPane->addTabPane();
    GuiPane* optionsPane = tabPane->addTab("Options");
    GuiPane* advancedPane = tabPane->addTab("Advanced");

    optionsPane->setNewChildSize(400, -1, 150);
    m_resolution = optionsPane->addDropDownList("Resolution", Array<String>({"1 x 1", "32 x 32", "64 x 36", "180 x 180", "256 x 256", "320 x 180", "640 x 360", "360 x 640", "1280 x 720", "720 x 1280", "1920 x 1080"}));
    optionsPane->addNumberBox("Film sensitivity scaling", &m_radianceScale, "x", GuiTheme::LOG_SLIDER, 0.1f, 10.0f);
    optionsPane->addCheckBox("Use evt. map for last event", &m_options.useEnvironmentMapForLastScatteringEvent);

    GuiPane* pane2 = optionsPane->addPane("", GuiTheme::NO_PANE_STYLE);
    pane2->setNewChildSize(400, -1, 150);
    pane2->setPosition(Point2(500, 0));
    pane2->addNumberBox("Paths per pixel",        &m_options.raysPerPixel, "", GuiTheme::LOG_SLIDER, 1, 8192 * 2);
    pane2->addNumberBox("Max scattering events", &m_options.maxScatteringEvents, "", GuiTheme::LINEAR_SLIDER, 1, 10);
    GuiButton* b = pane2->addButton("Render", this, &App::onRender);
    optionsPane->pack();

    advancedPane->setNewChildSize(400, -1, 150);
    GuiNumberBox<float>* importanceBox = advancedPane->addNumberBox("Max importance", &m_options.maxImportanceSamplingWeight, "", GuiTheme::LOG_SLIDER, 0.5f, 100.0f);
    advancedPane->addNumberBox("Max incident radiance", &m_options.maxIncidentRadiance, "W/(m^2 sr)", GuiTheme::LOG_SLIDER, 1.0f, 1e6f);
    GuiPane* directPane = advancedPane->addPane("Direct Illumination (Area Lights)");
    directPane->setNewChildSize(400, -1, 100);
    directPane->addNumberBox("Next Event Estimation fraction", &m_options.areaLightDirectFraction, "", GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setCaptionWidth(200);
    directPane->addEnumClassRadioButtons("Sampling Method", Pointer<PathTracer::Options::LightSamplingMethod>(&m_options.samplingMethod), GuiTheme::TOOL_RADIO_BUTTON_STYLE);
    directPane->moveRightOf(importanceBox);
    directPane->moveBy(0, 10);
    b = advancedPane->addButton("Render Convergence", this, &App::onRenderConvergence);
    advancedPane->addButton("Render All Scenes", this, &App::onBatchRender)->moveRightOf(b);
    advancedPane->pack();

    tabPane->pack();

#   ifdef G3D_DEBUG
        m_resolution->setSelectedIndex(2);
#   else
        m_resolution->setSelectedIndex(6);
#   endif

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


Vector2int32 App::resolution() const {
    return Vector2int32::parseResolution(m_resolution->selectedValue());
}


shared_ptr<Texture> App::renderOneImage(const PathTracer::Options& options, float& time, shared_ptr<Texture>& hdrImage) {
    
    const Vector2int32 res = resolution();
    drawMessage(format("Rendering %s", scene()->name().c_str()));

    m_pathTracer->setScene(scene());
    const shared_ptr<Image>& image = Image::create(res.x, res.y, ImageFormat::RGB32F());

    Stopwatch timer;
    timer.tick();
    m_pathTracer->traceImage(image, activeCamera(), options, [this](const String& msg, float pct) {
        debugPrintf("%d%% (%s)\n", iRound(100.0f * pct), msg.c_str());
    });
    timer.tock();

    hdrImage = Texture::fromImage("Source", image);

    FilmSettings filmSettings = activeCamera()->filmSettings();
    filmSettings.setSensitivity(filmSettings.sensitivity() * m_radianceScale);
    filmSettings.setTemporalAntialiasingEnabled(false);

    shared_ptr<Texture> dst;
    m_film->exposeAndRender(renderDevice, filmSettings, hdrImage, 0, 0, dst);
    dst->setCaption(format("\"%s\" @ %dx%d, %d spp, %d bounces in %d s", scene()->name().c_str(), dst->width(), dst->height(), options.raysPerPixel, options.maxScatteringEvents, iRound(timer.elapsedTime())));
    time = float(timer.elapsedTime());

    return dst;
}


void App::onRender() {
    float renderTime = 0;
    shared_ptr<Texture> hdrImage;
    const shared_ptr<Texture>& dst = renderOneImage(m_options, renderTime, hdrImage);
    show(hdrImage, "Raw Radiance");
    show(dst, format("%ds @ \n", iRound(renderTime)) + System::currentTimeString());
}


void App::onBatchRender() {
    Array<String> sceneNameArray(
        "G3D Simple Cornell Box (Water)",
        "G3D Simple Cornell Box (Spheres)",
        "G3D Simple Cornell Box (Area Light)",
        "G3D Sports Car");
    sceneNameArray.append(
        "G3D Sponza (Area Light)",
        "G3D Debug Roughness",
        "G3D Living Room (Area Lights)",
        "G3D Debug Depth of Field");

    PathTracer::Options options;
    Array<shared_ptr<Texture>> resultArray;
    for (const String& name : sceneNameArray) {
        shared_ptr<Texture> ignore;
        loadScene(name);
        float time = 0.0f;
        resultArray.append(renderOneImage(options, time, ignore));        
    }

    screenCapture()->saveImageGridToJournal("Results", resultArray);
}


void App::onRenderConvergence() {
    PathTracer::Options options = m_options;

    for (options.raysPerPixel = 1; options.raysPerPixel < m_options.raysPerPixel;
        options.raysPerPixel = max(m_options.raysPerPixel, iCeil(options.raysPerPixel * 2.0f))) {
        float renderTime = 0;

        shared_ptr<Texture> ignore;
        const shared_ptr<Texture>& dst = renderOneImage(options, renderTime, ignore);
        const shared_ptr<Image>& result = dst->toImage(ImageFormat::RGB8());
        result->save(FilePath::makeLegalFilename(dst->caption()) + ".png");
    }
}
