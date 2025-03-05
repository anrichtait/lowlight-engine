/** \file primaryRayTrace/main.cpp 

TODO: TextureBrowserWindow needs to know about high DPI

*/
#include <G3D/G3D.h>

G3D_DECLARE_ENUM_CLASS(ProjectionAlgorithm, PLANAR, FISHEYE, LENSLET, PANNINI, OCTAHEDRAL, THIN_LENS, CUBE_MAP, ORTHOGRAPHIC);

class App : public GApp {
protected:
    
    ProjectionAlgorithm                      m_projectionAlgorithm = ProjectionAlgorithm::PLANAR;

    /** World space ray origins and t_min */
    shared_ptr<Texture>                      m_rayOriginTexture;

    /** World space ray directions and t_max */
    shared_ptr<Texture>                      m_rayDirectionTexture;

    bool                                     m_accumulate = false;
    shared_ptr<Texture>                      m_accumulateTexture;

    bool                                     m_subPixelJitter = false;

    int                                      m_frameIndex = 0;

    //Camera parameters
    float                                   m_csProjectionOffset = 1.0f;
    float                                   m_verticalCompression = 0.0f;
    float                                   m_FOVDistance = 5.0f;
    bool                                    m_randomizeLensPoint = false;

    // Thins lens - physical parameters
    float                                   m_focalLength = -1;
    float                                   m_camToLens = -1;
    float                                   m_fStop = -1;

    float                                   m_focusDistance = 0.0f;

    virtual void makeGUI();

    void computeLensParameters(float lensRadius, float focusPlaneZ);

public:
    App(const GApp::Settings& settings) : GApp(settings) {}

    virtual void onInit() override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
    virtual void onAfterLoadScene(const Any& any, const String& sceneName) override;
};


void App::computeLensParameters(float lensRadius, float focusPlaneZ) {
    m_focalLength = abs(focusPlaneZ) / abs(focusPlaneZ + 1.0f);
    m_fStop = abs(m_focalLength / (2.0f * lensRadius));
    m_camToLens = 1.0f;
}

void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
    // Disable TAA
    activeCamera()->filmSettings().setTemporalAntialiasingEnabled(false);

    //////////////////////////////////////////////////////////////////////////////////

    const Vector2int32 size(rd->window()->width(), rd->window()->height());
    // (re)Allocate primary ray buffers
    if (isNull(m_rayDirectionTexture)) {
        m_rayOriginTexture    = Texture::createEmpty("m_rayOriginTexture", size.x, size.y, ImageFormat::RGBA32F());
        m_rayDirectionTexture = Texture::createEmpty("m_rayDirectionTexture", size.x, size.y, ImageFormat::RGBA32F());
    }
    m_rayOriginTexture->resize(size.x, size.y);
    m_rayDirectionTexture->resize(size.x, size.y);

    BEGIN_PROFILER_EVENT("Ray Generation");
    const Rect2D viewport = Rect2D::xywh(0.0f, 0.0f, (float)size.x, (float)size.y);
    {
        Args args;
        args.setRect(viewport);

        const Vector3int32 blockSize(16, 16, 1);
        activeCamera()->setShaderArgs(args, viewport.wh(), "camera.");
        args.setMacro("PROJECTION_ALGORITHM", m_projectionAlgorithm);
        
        // Camera parameters.
        args.setUniform("csProjectionOffset", m_csProjectionOffset);
        args.setUniform("verticalCompression", m_verticalCompression);
        args.setUniform("FOVRadians", activeCamera()->projection().fieldOfViewAngle());
        args.setUniform("FOVDirection", activeCamera()->projection().fieldOfViewDirection());
        args.setUniform("FOVDistance", m_FOVDistance);

        if (m_focalLength == -1.0f) {
            computeLensParameters(activeCamera()->depthOfFieldSettings().lensRadius(), activeCamera()->depthOfFieldSettings().focusPlaneZ());
        }

        // Thin lens parameters
        args.setUniform("focalLength", m_focalLength);
        args.setUniform("camToLens", m_camToLens);
        args.setUniform("fStop", m_fStop);


        Vector2 lensPoint = Vector2(0.0f, 0.0f);
        if (m_randomizeLensPoint) {
            lensPoint = Vector2(HaltonSequence::sample(m_frameIndex, 5), HaltonSequence::sample(m_frameIndex, 7));
        }
        args.setUniform("lensPoint", lensPoint);

        Vector2 offset = Vector2(0.5f, 0.5f);
        if (m_subPixelJitter) {
            offset = Vector2(HaltonSequence::sample(m_frameIndex, 2), HaltonSequence::sample(m_frameIndex, 3));
        }
        args.setUniform("pixelOffset", offset);

        args.setImageUniform("rayOrigin", m_rayOriginTexture, Access::WRITE);
        args.setImageUniform("rayDirection", m_rayDirectionTexture, Access::WRITE);

        args.setComputeGridDim(Vector3int32(iCeil(viewport.width() / float(blockSize.x)), iCeil(viewport.height() / float(blockSize.y)), 1));
        args.setComputeGroupSize(blockSize);
        
        debugAssertGLOk();
        LAUNCH_SHADER("generateRays.glc", args);
    }
    END_PROFILER_EVENT();

    //////////////////////////////////////////////////////////////////////////////////
    // Cast primary rays, storing results in a non-coherent GBuffer
    m_gbuffer->prepare(rd, activeCamera(), 0, -(float)previousSimTimeStep(), Vector2int16(), Vector2int16());

    // Using Textures and GBuffer directly is slower than the PixelTransferBuffer interface for
    // TriTree, but easier to use for a simple program.
    scene()->tritree()->intersectRays(m_rayOriginTexture, m_rayDirectionTexture, m_gbuffer);

    //////////////////////////////////////////////////////////////////////////////////
    // Compute shadow maps
    Light::renderShadowMaps(rd, scene()->lightingEnvironment().lightArray, surface3D);

    // Find the skybox
    shared_ptr<SkyboxSurface> skyboxSurface;
    for (const shared_ptr<Surface>& surface : surface3D) {
        skyboxSurface = dynamic_pointer_cast<SkyboxSurface>(surface);
        if (skyboxSurface) { break; }
    }

    //////////////////////////////////////////////////////////////////////////////////
    // Perform deferred shading on the GBuffer
    rd->push2D(m_framebuffer); {
        // Disable screen-space effects
        LightingEnvironment e = scene()->lightingEnvironment();

        Args args;
        e.ambientOcclusionSettings.enabled = false;
        e.setShaderArgs(args);
        args.setMacro("OVERRIDE_SKYBOX", true);
        args.setMacro("COMPUTE_PERCENT", 0);
        if (skyboxSurface) {
            skyboxSurface->setShaderArgs(args, "skybox_");
        }
        m_gbuffer->setShaderArgsRead(args, "gbuffer_");

        m_rayDirectionTexture->setShaderArgs(args, "gbuffer_WS_RAY_DIRECTION_", Sampler::buffer());
        // Set the color for degenerate rays so the shaders knows not to sample the skybox.
        args.setMacro("DEGENERATE_RAY_COLOR", Color3(0.0f));

        args.setRect(rd->viewport());

        LAUNCH_SHADER("DefaultRenderer/DefaultRenderer_deferredShade.pix", args);
    } rd->pop2D();

    if (m_accumulate) {
        if (isNull(m_accumulateTexture)) {
            m_accumulateTexture = Texture::createEmpty("accumulatedFrame", m_framebuffer->texture(0)->width(), m_framebuffer->texture(0)->height(), m_framebuffer->texture(0)->format());
        }

        Args args;
        args.setRect(rd->viewport());

        const Vector3int32 blockSize(16, 16, 1);

        args.setImageUniform("newFrame", m_framebuffer->texture(0), Access::READ);
        args.setImageUniform("accumulatedFrame", m_accumulateTexture, Access::READ_WRITE);

        args.setUniform("numFrames", m_frameIndex);

        args.setComputeGridDim(Vector3int32(iCeil(viewport.width() / float(blockSize.x)), iCeil(viewport.height() / float(blockSize.y)), 1));
        args.setComputeGroupSize(blockSize);

        debugAssertGLOk();
        LAUNCH_SHADER("accumulate.glc", args);
    }

    // TODO: frameIndex should reset when the camera moves or the scene changes.
    if (m_subPixelJitter || m_randomizeLensPoint) {
        ++m_frameIndex;
    }
    else {
        m_frameIndex = 0;
    }


    swapBuffers();
    rd->clear();

    // Disable all positional effects
    FilmSettings postSettings = activeCamera()->filmSettings();
    postSettings.setAntialiasingEnabled(true);
    postSettings.setTemporalAntialiasingEnabled(false);
    postSettings.setVignetteBottomStrength(0);
    postSettings.setVignetteTopStrength(0);
    postSettings.setBloomStrength(0);
    m_film->exposeAndRender(rd, postSettings, m_accumulate ? m_accumulateTexture : m_framebuffer->texture(0), settings().hdrFramebuffer.trimBandThickness().x, settings().hdrFramebuffer.depthGuardBandThickness.x);
}


void App::onAfterLoadScene(const Any& any, const String& sceneName) {
    GApp::onAfterLoadScene(any, sceneName);
    // Disable TAA and postFX
    FilmSettings& postSettings = activeCamera()->filmSettings();
    postSettings.setTemporalAntialiasingEnabled(false);
    postSettings.setAntialiasingEnabled(false);
    postSettings.setTemporalAntialiasingEnabled(false);
    postSettings.setVignetteBottomStrength(0);
    postSettings.setVignetteTopStrength(0);
    postSettings.setBloomStrength(0);

    if (sceneName == "Figure Greek Temple") {
        scene()->setTime(12);
        setSimulationTimeScale(0.0f);
    }
}


void App::onInit() {
    GApp::onInit();         

    setFrameDuration(1.0f / 1000.0f, -200.0f);

    loadScene(
#       ifndef G3D_DEBUG
        //"Figure Greek Temple"
        "G3D Sponza"
        //"G3D Sibenik (Projection)"
#       else
        "G3D Simple Cornell Box (Area Light)"
        //"G3D Simple Motion"
#       endif
    );

    makeGUI();
}


void App::makeGUI() {
    developerWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    debugWindow->setVisible(true);

    developerWindow->sceneEditorWindow->setSimulationPaused(true);

    const float PANEL_WIDTH = 300;
    // Resize the debugWindow AND the debugPane
    debugWindow->setRect(Rect2D::xywh(0, 0, PANEL_WIDTH, float(window()->height())));
    debugPane->setRect(Rect2D::xywh(0, 0, PANEL_WIDTH, float(window()->height())));

    //debugCamera()->setFieldOfViewDirection(FOVDirection::HORIZONTAL);
    //debugCamera()->setFieldOfViewAngle(toRadians(150));
    showRenderingStats = false;
    setActiveCamera(m_debugCamera);

    debugPane->addEnumClassRadioButtons<ProjectionAlgorithm>("ProjectionAlgorithm", &m_projectionAlgorithm, GuiTheme::BUTTON_RADIO_BUTTON_STYLE);
    debugPane->addCheckBox("Accumulate",
        Pointer<bool>([&]() {
            return m_accumulate;
            },
            [&](bool b) {
                if (!m_accumulate && b) {
                    m_frameIndex = 0;
                }
                m_accumulate = b;
                m_subPixelJitter = b;
            }));
    debugPane->addCheckBox("Sub-pixel jitter", &m_subPixelJitter);

    { // Camera pane

        GuiPane* cameraPane = debugPane->addPane("Camera Parameters", GuiTheme::PaneStyle::ORNATE_PANE_STYLE);

        const float PARAM_WIDTH = 250;
        GuiNumberBox<float>* b = cameraPane->addNumberBox("Near Plane Z",
            Pointer<float>(activeCamera(), &Camera::nearPlaneZ, &Camera::setNearPlaneZ),
            "m", GuiTheme::LOG_SLIDER, -80.0f, -0.001f);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        b = cameraPane->addNumberBox("Far Plane Z", Pointer<float>(activeCamera(), &Camera::farPlaneZ, &Camera::setFarPlaneZ), "m", GuiTheme::LOG_SLIDER, -1000.0f, -0.10f, 0.0f, GuiTheme::NORMAL_TEXT_BOX_STYLE, true, false);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        b = cameraPane->addNumberBox("Field of View", Pointer<float>(activeCamera(), &Camera::fieldOfViewAngleDegrees, &Camera::setFieldOfViewAngleDegrees), "", GuiTheme::LOG_SLIDER, 10.0f, 360.0f, 0.5f);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        Pointer<FOVDirection> directionPtr(activeCamera(), &Camera::fieldOfViewDirection, &Camera::setFieldOfViewDirection);
        GuiRadioButton* radioButton2 = cameraPane->addRadioButton("Horizontal", FOVDirection::HORIZONTAL, directionPtr, GuiTheme::BUTTON_RADIO_BUTTON_STYLE);
        radioButton2->setWidth(91);
        GuiRadioButton* radioButton;
        radioButton = cameraPane->addRadioButton("Vertical", FOVDirection::VERTICAL, directionPtr, GuiTheme::BUTTON_RADIO_BUTTON_STYLE);
        radioButton->setWidth(radioButton2->rect().width());

        radioButton = cameraPane->addRadioButton("Diagonal", FOVDirection::DIAGONAL, directionPtr, GuiTheme::BUTTON_RADIO_BUTTON_STYLE);
        radioButton->setWidth(radioButton2->rect().width());

        GuiPane* panniniPane = cameraPane->addPane("Pannini");
        b = panniniPane->addNumberBox("Proj. Offset", &m_csProjectionOffset, "", GuiTheme::LINEAR_SLIDER, 0.0f, 1000.0f);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        b = panniniPane->addNumberBox("Vert. Compression", &m_verticalCompression, "", GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        b = cameraPane->addNumberBox("FOV Distance", &m_FOVDistance, "", GuiTheme::LINEAR_SLIDER, 0.0f, 10.0f);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        GuiPane* thinLensPane = cameraPane->addPane("Thin Lens 1");

        b = thinLensPane->addNumberBox
        ("Focus Distance", &m_focusDistance,
            "m", GuiTheme::LINEAR_SLIDER, 0.01f, 200.0f);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        b = thinLensPane->addNumberBox
        ("Lens Radius",
            Pointer<float>(
                [&]() {
                    return (m_focalLength / m_fStop);
                },
                [&](float f) {
                    return; // read only
                }),
            "m", GuiTheme::LOG_SLIDER, 0.0f, 0.05f);
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        GuiPane* thinLens2Pane = cameraPane->addPane("Thin Lens 2");
        b = thinLens2Pane->addNumberBox
        ("Dist. to Lens",
            Pointer<float>(
                [&]() {
                    return m_camToLens;
                },
                [&](float f) {
                    m_camToLens = f;
                    m_focusDistance = m_camToLens * m_focalLength / (m_camToLens - m_focalLength);
                }),
            "m", GuiTheme::LOG_SLIDER, 0.0f, 10.0f); // TODO: this should go from 0 to the lens focal length.
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        b = thinLens2Pane->addNumberBox
        ("Focal Length",
            Pointer<float>(
                [&]() {
                    return m_focalLength;
                },
                [&](float f) {
                    m_focalLength = f;
                    m_focusDistance = m_camToLens * m_focalLength / (m_camToLens - m_focalLength);
                }),
            "m", GuiTheme::LOG_SLIDER, 0.0f, 10.0f); // TODO: this should go from 0 to the lens focal length.
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);

        b = thinLens2Pane->addNumberBox
        ("F-stop",
            &m_fStop,
            "", GuiTheme::LOG_SLIDER, 0.0f, 10.0f); // TODO: this should go from 0 to the lens focal length.
        b->setWidth(PARAM_WIDTH);  b->setCaptionWidth(105);


        thinLens2Pane->addCheckBox("Randomize Lens Point", &m_randomizeLensPoint);
    }
}


G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    initGLG3D();

    GApp::Settings settings(argc, argv);  
    settings.window.caption = "Primary Ray Tracing";
    settings.window.width = OSWindow::primaryDisplayWindowSize().x;
    settings.window.height = OSWindow::primaryDisplayWindowSize().y;
    settings.window.resizable = true;
    settings.window.refreshRate = -1;
    settings.window.asynchronous = true;
    //settings.window.resizable = true;

    // Set high fidelity format for correct accumulation.
    Array<const ImageFormat*> format = { ImageFormat::RGBA32F() };
    settings.hdrFramebuffer.preferredColorFormats = format;

    settings.hdrFramebuffer.colorGuardBandThickness = settings.hdrFramebuffer.depthGuardBandThickness  = Vector2int16(0, 0);

    return App(settings).run();
}
