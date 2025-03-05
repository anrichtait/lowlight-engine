/** \file App.cpp */
#include "App.h"

G3D_START_AT_MAIN();

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    GApp::Settings settings;
    
    settings.window.width       = 1280; 
    settings.window.height      = 720;
    settings.window.caption     = "All Stages Shader Demo";

#   ifdef G3D_WINDOWS
        // On Unix operating systems, icompile automatically copies data files.  
        // On Windows, we just run from the data directory.
        if (FileSystem::exists("data-files")) {
            chdir("data-files");
        } else if (FileSystem::exists("../samples/allStagesShader/data-files")) {
            chdir("../samples/allStagesShader/data-files");
        }

#   endif

    return App(settings).run();
}

App::App(const GApp::Settings& settings) : GApp(settings), m_innerTessLevel(1.0f), m_outerTessLevel(1.0f) {}

void App::makeGUI() {
    debugWindow->setVisible(true);
    developerWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    showDebugText = false;
    showRenderingStats = false;
    
    GuiNumberBox<float>* innerSlider = debugPane->addNumberBox("Inner Tesselation Level", &m_innerTessLevel, "", GuiTheme::LINEAR_SLIDER, 1.0f, 20.f);
    innerSlider->setWidth(290.0f);
    innerSlider->setCaptionWidth(140.0f);
    GuiNumberBox<float>* outerSlider = debugPane->addNumberBox("Outer Tesselation Level", &m_outerTessLevel, "", GuiTheme::LINEAR_SLIDER, 1.0f, 20.f);
    outerSlider->setCaptionWidth(140.0f);
    outerSlider->setWidth(290.0f);
    debugPane->pack();
    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, window()->height()-debugWindow->rect().height(), 300, debugWindow->rect().height()));
}


void App::onInit() {
    GApp::onInit();
    renderDevice->setSwapBuffersAutomatically(true);

    ArticulatedModel::fromFile(System::findDataFile("icosahedron/icosahedron.obj"))->pose(m_sceneGeometry, CFrame(), CFrame(), nullptr, nullptr, nullptr, Surface::ExpressiveLightScatteringProperties());
    m_allStagesShader = Shader::fromFiles("geodesic.vrt", "geodesic.ctl", "geodesic.evl", "geodesic.geo", "geodesic.pix");

    makeGUI();
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
    // Bind the main framebuffer
    rd->pushState(m_framebuffer); {

        rd->setColorClearValue(Color3::white() * 0.3f);
        rd->clear();
        
        
        rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);
        Args args;
        args.setUniform("TessLevelInner", m_innerTessLevel);
        args.setUniform("TessLevelOuter", m_outerTessLevel);
        args.setPrimitiveType(PrimitiveType::PATCHES);
        args.patchVertices = 3;
        rd->setDepthTest(RenderDevice::DEPTH_LEQUAL);
        rd->setProjectionAndCameraMatrix(m_debugCamera->projection(), m_debugCamera->frame());
        
        for (int i = 0; i < m_sceneGeometry.size(); ++i) {
            const shared_ptr<UniversalSurface>&  surface = dynamic_pointer_cast<UniversalSurface>(m_sceneGeometry[i]);
            if (isNull(surface)) {
                debugPrintf("Surface %d, not a supersurface.\n", i);
                continue;
            }
            const shared_ptr<UniversalSurface::GPUGeom>& gpuGeom = surface->gpuGeom();
            args.setAttributeArray("Position", gpuGeom->vertex);
            args.setIndexStream(gpuGeom->index);
            
            CoordinateFrame cf;
            surface->getCoordinateFrame(cf);
            rd->setObjectToWorldMatrix(cf);
            
            rd->apply(m_allStagesShader, args);
        }
        
    } rd->popState();
    
    // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
    m_film->exposeAndRender(rd, m_debugCamera->filmSettings(), m_framebuffer->texture(0), settings().hdrFramebuffer.trimBandThickness().x, settings().hdrFramebuffer.depthGuardBandThickness.x);
}

