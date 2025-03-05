#include <G3D/G3D.h>

#ifdef G3D_OSX
#    error "macOS does not support GLES"
#endif

G3D_START_AT_MAIN()

int main(int argc, const char** argv) {
    (void)argc; (void)argv;
    // Creating OSWindow::Settings directly makes the linker fail on
    // Raspberry Pi OS, so we create a GApp::Settings and then only
    // write to the OSWindow::Settings field of it.
    GApp::Settings settings;
    settings.window.api = OSWindow::Settings::API_OPENGL_EGL;

    OSWindow* window = OSWindow::create(settings.window);

    RenderDevice* renderDevice = new RenderDevice();
    renderDevice->init(window);

    fprintf(stderr, "GPU:  %s\nGLES: %s\nGLSL: %s\n",
        glGetString(GL_RENDERER),
        glGetString(GL_VERSION),
        glGetString(GL_SHADING_LANGUAGE_VERSION));


    Profiler::setEnabled(true);
    const shared_ptr<Texture>& texture = Texture::fromFile(System::findDataFile("gui/keyguide-small.png"));

    const shared_ptr<Framebuffer>& framebuffer = Framebuffer::create(Texture::createEmpty("Destination", 1024, 1024, ImageFormat::RGBA8()));

    const int N = 100;
    BEGIN_PROFILER_EVENT("everything");
    renderDevice->push2D(framebuffer);
    renderDevice->clear();
    for (int i = 0; i < N; ++i) {
        Args args;
        args.setUniform("texture", texture, Sampler::defaults());
        args.setRect(renderDevice->viewport());
        LAUNCH_SHADER("test.pix", args);
    }
    renderDevice->pop2D();
    END_PROFILER_EVENT();
    Profiler::nextFrame();

    // Read back the image to the CPU and save to disk
    framebuffer->texture(0)->toImage()->save("destination.png");

    // Read back the exetime
    RealTime cpuTime, gfxTime;
    Profiler::getEventTime("everything", cpuTime, gfxTime);
    debugPrintf("GPU Time for test.pix: %g ms / frame", gfxTime / (RealTime(N) * units::milliseconds()));

    renderDevice->cleanup();
    delete renderDevice;
    delete window;
    return 0;
}
