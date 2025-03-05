/** \file App.cpp */
#include "App.h"

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);
    settings.window.caption             = "Unsharp Masking Image Processing Example";
    settings.window.width               = 1024; settings.window.height       = 720;
    settings.window.fullScreen          = false;
    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(0, 0);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir                    = FileSystem::currentDirectory();

    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {
}


void App::onInit() {
    GApp::onInit();
    renderDevice->setSwapBuffersAutomatically(true);
    m_source = Texture::fromFile("blurry.png", ImageFormat::RGB32F(), Texture::DIM_2D, false);

    makeGUI();
    onParameterChange();
}


bool App::onEvent(const GEvent& event) {
    if (GApp::onEvent(event)) {
        return true;
    } else if ((event.type == GEventType::GUI_ACTION) && (event.gui.control == m_sigmaSlider)) {
        // Handle the slider changing
        onParameterChange();
        return true;
    } else {
        return false;
    }
}


void App::onParameterChange() {
    // Initialize and size the output as needed
    if (isNull(m_destination)) {
        m_destination = Framebuffer::create(Texture::createEmpty("m_destination", 128, 128, ImageFormat::RGB32F()));
    }

    m_destination->resize(m_source->vector2Bounds());

    RenderDevice* rd = renderDevice;

    // Bind the output framebuffer
    rd->push2D(m_destination); {

        Args args;

        // We will only use texelFetch and pixel coordinates in this example, so
        // Sampler::buffer is the ideal sampling mode.
        // 
        // If you plan to use texture(), textureLod(), etc. in GLSL, look at
        // Sampler::defaults(), Sampler::defaultClamp(), and Sampler::video()
        // for interpolation modes that you might prefer.
        args.setUniform("source", m_source, Sampler::buffer());

        // Pass a scalar argument to the shader
        args.setUniform("sigma", m_sigma);
        
        // Target every pixel of the output
        args.setRect(rd->viewport());
        LAUNCH_SHADER("unsharpMask.pix", args);
    } rd->pop2D();
    
}


void App::makeGUI() {
    debugWindow->setVisible(true);
    showRenderingStats      = false;
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->setVisible(false);

    debugPane->beginRow(); {
        m_sigmaSlider = debugPane->addNumberBox("Unsharp radius", &m_sigma, "pix", GuiTheme::LINEAR_SLIDER, 0.5f, 10.0f);
        m_sigmaSlider->setWidth(300);
        m_sigmaSlider->setCaptionWidth(100);
        
        debugPane->addCheckBox("Show source", &m_showSource)->moveBy(40, 0);

    } debugPane->endRow();

    // More examples of debugging GUI controls:
    // debugPane->addCheckBox("Use explicit checking", &explicitCheck);
    // debugPane->addTextBox("Name", &myName);
    // debugPane->addNumberBox("height", &height, "m", GuiTheme::LINEAR_SLIDER, 1.0f, 2.5f);
    // button = debugPane->addButton("Run Simulator");
    // debugPane->addButton("Generate Heightfield", [this](){ generateHeightfield(); });
    // debugPane->addButton("Generate Heightfield", [this](){ makeHeightfield(imageName, scale, "model/heightfield.off"); });

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    // There is no 3D in this program, so just override to draw a white background
    rd->setColorClearValue(Color3::white());
    rd->clear();
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    // Draw the processed image
    Draw::rect2D(rd->viewport().largestCenteredSubRect((float)m_destination->width(), (float)m_destination->height()),
                 rd, Color3::white(), m_showSource ? m_source : m_destination->texture(0), Sampler::video(), false);
    
    // Render 2D objects like Widgets.  These do not receive tone mapping or gamma correction.
    Surface2D::sortAndRender(rd, posed2D);
}
