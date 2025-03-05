/** \file App.cpp */
#include "App.h"

G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    (void)argc; (void)argv;
    GApp::Settings settings(argc, argv);
    settings.window.width       = 512; 
    settings.window.height      = 512;
    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {}


void App::onInit() {
    GApp::onInit();
    developerWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);

    // load the texture_2D_Array from files
    causticTexture = Texture::fromFile(System::findDataFile("gobo/waterCaustic/waterCaustic_001.jpg"), ImageFormat::SRGB8(), Texture::Dimension::DIM_2D_ARRAY);
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
    rd->push2D(m_framebuffer); {
        Args args;
        args.setUniform("textureArray", causticTexture, Sampler::buffer());
        args.setUniform("bounds", m_framebuffer->vector2Bounds());
        args.setRect(m_framebuffer->rect2DBounds());
        LAUNCH_SHADER("TextureArraySample.pix", args);
    } rd->pop2D();
    swapBuffers();
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), settings().hdrFramebuffer.trimBandThickness().x,
        settings().hdrFramebuffer.depthGuardBandThickness.x);
}
