/** \file main.cpp */
#include <G3D/G3D.h>

typedef
    GApp 
    // VRApp
AppBase;

class App : public AppBase {
protected:

    /** RGBA clouds at 1/3 resolution */
    shared_ptr<Framebuffer>     m_cloudFramebuffer;
    
    /** RGB = HDR color, A = depth */
    shared_ptr<Framebuffer>     m_planetFramebuffer;

public:

    typedef AppBase super;
    
    App(const VRApp::Settings& settings = VRApp::Settings());

    virtual void onInit() override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
};


App::App(const VRApp::Settings& settings) : super(settings) {}


void App::onInit() {
    super::onInit();
    setFrameDuration(1.0f / 60.0f);
    showRenderingStats  = false;
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));

    const int w = renderDevice->width(), h = renderDevice->height();
    m_planetFramebuffer = Framebuffer::create(Texture::createEmpty("m_planetFramebuffer::Color", w, h, ImageFormat::RGBA16F()));
    m_cloudFramebuffer = Framebuffer::create(Texture::createEmpty("m_cloudFramebuffer::Color", w/3, h/3, ImageFormat::RGBA8()));

    // Just load the camera settings
    loadScene("Camera");
    m_debugController->setMoveRate(0.2f);
    setActiveCamera(m_debugCamera);
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    m_cloudFramebuffer->resize(m_framebuffer->width() / 3, m_framebuffer->height() / 3);
    m_planetFramebuffer->resize(m_framebuffer->width(), m_framebuffer->height());

    rd->push2D(m_planetFramebuffer); {
        rd->setAlphaWrite(true);
        Args args;
        args.setUniform("ambientCubeMap", scene()->lightingEnvironment().environmentMapArray[0], Sampler::cubeMap());
        args.setRect(rd->viewport());
        args.setUniform("iMouse", userInput->mouseXY());
        args.setUniform("iMouseVelocity", userInput->mouseDXY());
        LAUNCH_SHADER("planet.pix", args);
    } rd->pop2D();

    rd->push2D(m_cloudFramebuffer); {
        rd->setAlphaWrite(true);
        Args args;
        args.setUniform("planetTexture", m_planetFramebuffer->texture(0), Sampler::buffer());
        args.setUniform("iMouse", userInput->mouseXY());
        args.setUniform("iMouseVelocity", userInput->mouseDXY());
        args.setRect(rd->viewport());
        LAUNCH_SHADER("clouds.pix", args);
    } rd->pop2D();

    rd->push2D(m_framebuffer); {
        Args args;
        args.setUniform("planetTexture", m_planetFramebuffer->texture(0), Sampler::buffer());
        args.setUniform("cloudTexture", m_cloudFramebuffer->texture(0), Sampler::video());
        args.setUniform("previousFrameTexture", m_framebuffer->texture(0), Sampler::buffer());  
        args.setUniform("iMouse", userInput->mouseXY());
        args.setUniform("iMouseVelocity", userInput->mouseDXY());

        args.setRect(rd->viewport());
        LAUNCH_SHADER("composite.pix", args);
    } rd->pop2D();

    swapBuffers();

    rd->clear();
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0),
        settings().hdrFramebuffer.trimBandThickness().x,
        settings().hdrFramebuffer.depthGuardBandThickness.x);
}


G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    VRApp::Settings settings(argc, argv);

    settings.window.caption             = "G3D Implicit Planet Sample";
    settings.window.width               = 1200; settings.window.height       = 650;

    // Shadertoy small window size:
    //settings.window.width               = 560; settings.window.height       = 320;

    settings.window.fullScreen          = false;
    settings.window.resizable           = ! settings.window.fullScreen;
    settings.window.framed              = ! settings.window.fullScreen;
    settings.window.asynchronous        = true;
    settings.hdrFramebuffer.depthGuardBandThickness    = Vector2int16(0, 0);
    settings.hdrFramebuffer.colorGuardBandThickness    = Vector2int16(0, 0);
    settings.dataDir                    = FileSystem::currentDirectory();

    return App(settings).run();
}