#include <G3D/G3D.h>

class App : public GApp {
protected:

    RealTime                    m_startTime = 0;
    RealTime                    m_lastTime = 0;
    int                         m_frameNumber = 0;
    Vector4                     m_mouse;
    shared_ptr<Shader>          m_shader;
    shared_ptr<Framebuffer>     m_offscreen;

public:
    
    App(const GApp::Settings& settings) : GApp(settings) {}

    virtual void onInit() override {
        GApp::onInit();
        setFrameDuration(0.0f);
        setLowerFrameRateInBackground(false);
        developerWindow->sceneEditorWindow->setVisible(false);
        developerWindow->setVisible(false);
        developerWindow->cameraControlWindow->setVisible(false);
        renderDevice->setSwapBuffersAutomatically(true);

        const int w = renderDevice->width(), h = renderDevice->height();
        m_offscreen = Framebuffer::create(Texture::createEmpty("m_offscreen", w, h),
                                          Texture::createEmpty("depthAndStencil", w, h, ImageFormat::DEPTH24_STENCIL8()));
        loadShader(
            "flame.pix"
        );
    }
   
    void loadShader(const String& filename) {
        m_lastTime = m_startTime = System::time();
        m_frameNumber = 0;
        m_shader = Shader::fromFiles(filename);
    }

    virtual void onUserInput(UserInput* ui) override {
        GApp::onUserInput(ui);

        // Hold
        if (ui->keyDown(GKey::LEFT_MOUSE)) { (Vector2&)m_mouse = userInput->mouseXY(); }

        // Click
        if (ui->keyPressed(GKey::LEFT_MOUSE)) { (Vector2&)(m_mouse.z) = m_mouse.xy(); }
    }

    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) override {
        rd->push2D(m_offscreen); {
            rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);
            rd->setDepthWrite(false);
            Args args;
            args.setUniform("iFrame", m_frameNumber);
            args.setRect(rd->viewport());

            const float iTime = float(System::time() - m_startTime);
            args.setUniform("iTime", iTime);
            args.setUniform("iTimeDelta", iTime - m_lastTime);
            args.setUniform("iMouse", m_mouse);
            rd->setColorWrite(true);
            LAUNCH_SHADER_PTR(m_shader, args);
            m_lastTime = iTime;
        } rd->pop2D();
        ++m_frameNumber;

        Draw::rect2D(rd->viewport(), rd, Color3::white(), m_offscreen->texture(0), Sampler::buffer(), true);            
        Surface2D::sortAndRender(rd, posed2D);
    }

    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface>>& surface3D) override { }
};

G3D_START_AT_MAIN();
int main(int argc, const char* argv[]) {
    const int scale = 3;
    VRApp::Settings settings(argc, argv);
    settings.window.caption = "G3D Shadertoy";
    settings.window.width = 560 * scale; settings.window.height = 320 * scale;
    settings.dataDir = FileSystem::currentDirectory();
    return App(settings).run();
}
