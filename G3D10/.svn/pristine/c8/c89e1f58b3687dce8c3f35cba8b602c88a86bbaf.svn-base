/** \file App.cpp */
#include "App.h"
#include "WireMesh.h"

// Set your monitor's desktop refresh rate (e.g., in the NVIDIA Control Panel)
// to the highest rate that it supports before running this program.

// Set to -1 for the highest frame rate available on this monitor,
// or a value in frames per second (Hz) to lock a specific rate.
static const float targetFrameRate            = 240; // Hz

// Enable this to see maximum CPU/GPU rate when not limited
// by the monitor. 
static const bool  unlockFramerate            = false;

// Set to true if the monitor has G-SYNC/Adaptive VSync/FreeSync, 
// which allows the application to submit asynchronously with vsync
// without tearing.
static const bool  variableRefreshRate        = true;

static const float verticalFieldOfViewDegrees = 90; // deg

// Set to false when debugging
static const bool  playMode                   = true;


App::App(const GApp::Settings& settings) : GApp(settings) {
}


void App::onInit() {
    GApp::onInit();
    m_font = GFont::fromFile(System::findDataFile("arial.fnt"));

    float dt = 0;

    if (unlockFramerate) {
        dt = 1.0f / 5000.0f;
    } else if (variableRefreshRate) {
        dt = 1.0f / targetFrameRate;
    } else {
        dt = 1.0f / float(window()->settings().refreshRate);
    }

    setFrameDuration(dt);
    renderDevice->setColorClearValue(Color3::white() * 0.0f);
    debugCamera()->setFrame(Point3(-5, -2, 0));
    debugCamera()->projection().setFieldOfViewAngleDegrees(verticalFieldOfViewDegrees);
    m_debugController->setFrame(debugCamera()->frame());

    if (playMode) {
        const shared_ptr<FirstPersonManipulator>& fpm = dynamic_pointer_cast<FirstPersonManipulator>(cameraManipulator());
        fpm->setMouseMode(FirstPersonManipulator::MOUSE_DIRECT);
        fpm->setMoveRate(0.0);
    }

    Projection& P = debugCamera()->projection();
    P.setFarPlaneZ(-finf());

    debugWindow->setVisible(false);
    developerWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    showRenderingStats = false;

    m_targetMesh = WireMesh::create(System::findDataFile("ifs/d20.ifs"), 1.0, Color3::blue(), Color3::orange());

    m_targetArray.append(
        Target(CFrame::fromXYZYPRDegrees(3,2,-8, 0.0f, 0.0f, 0.0f),
               CFrame::fromXYZYPRDegrees(0.0f, 0.0f, 0.0f, 10.0f * dt, -7.0f * dt, 0)),

        Target(CFrame::fromXYZYPRDegrees(-2.0f, -0.5f,-15.0f , 40.0f, 0.0f, 10.0f),
               CFrame::fromXYZYPRDegrees(0.0f, 0.0f, 0.0f, -5.0f * dt, 40.0f * dt, 0.0f)));
}


bool App::onEvent(const GEvent& e) {
    if (GApp::onEvent(e)) {
        return true;
    }

    if (e.type == GEventType::MOUSE_BUTTON_DOWN) {
        const Ray& wsRay = activeCamera()->worldRay(floor(m_framebuffer->width() / 2.0f) + 0.5f,
            floor(m_framebuffer->height() / 2.0f) + 0.5f, m_framebuffer->rect2DBounds());
        
        //debugDraw(std::make_shared<SphereShape>(wsRay.origin() + wsRay.direction() * 2.0f, 0.1f), -1);

        // Find first hit
        int hitIndex = -1;
        float hitDistance = finf();
        for (int i = 0; i < m_targetArray.size(); ++i) {
            const Target& target = m_targetArray[i];
            float t = wsRay.intersectionTime(Sphere(target.cframe.translation, target.hitRadius));
            if (t < hitDistance) {
                hitDistance = t;
                hitIndex = i;
            }
        }

        if (hitIndex != -1) {
            // Process the hit
            m_targetArray.fastRemove(hitIndex);
        }
    }

    return false;
}


/** Make objects fade towards black with distance as a depth cue */
static float distanceDarken(const float csZ) {
    const float t = max(0.0f, abs(csZ) - 10.0f);
    return exp(-t * 0.1f);
}


static Color3 computeTunnelColor(float alpha, float angle) {
    const float a = clamp(1.0f - abs(alpha), 0.0f, 1.0f);

    static const Color3 pink(1.0f, 0.25f, 0.25f);
    const float c = abs(wrap(0.25f + angle / (2.0f * pif()), 1.0f) - 0.5f) * 2.0f;
    const Color3 shade = Color3::cyan().lerp(pink, c);

    return (shade * distanceDarken(alpha * 100.0f)).pow(0.5f);
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    for (Target& target : m_targetArray) {
        target.cframe = target.cframe * target.velocity;
    }
}


void App::onPose(Array<shared_ptr<Surface>>& surface, Array<shared_ptr<Surface2D>>& surface2D) {
    GApp::onPose(surface, surface2D);

    m_posedMeshArray.fastClear();
    m_posedCFrameArray.fastClear();
    for (int t = 0; t < m_targetArray.size(); ++t) {
        m_posedMeshArray.append(m_targetMesh);
        m_posedCFrameArray.append(m_targetArray[t].cframe);
    }
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) {
    ////////////////////////////////////////////////////////////////////
    //                                                                //
    //                      Under construction!                       //
    //                                                                //
    //  This is actually quite slow. It is a gameplay prototype that  //
    //  will be replaced mid-November 2018 with the actual optimized  //
    //  code which produces similar visuals using optimal rendering.  //
    //                                                                //
    ////////////////////////////////////////////////////////////////////

    rd->swapBuffers();
    rd->clear();
    
    // Done by the caller for us:
    // rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());

    // For debugging
    // Draw::axes(Point3::zero(), rd);

    static const shared_ptr<Texture> reticleTexture = Texture::fromFile(System::findDataFile("gui/reticle/reticle-000.png"));
    static SlowMesh tunnelMesh(PrimitiveType::LINES);
    static bool first = true;

    if (first) { 
        // Tunnel
        const int axisSlices = 64;
        const int cylinderSlices = 12;
        const float radius = 12.0f;
        const float extent = 250.0f;

        for (int i = 0; i < axisSlices; ++i) {
            const float alpha = 2.0f * (float(i) / float(axisSlices - 1) - 0.5f);
            const float z = alpha * extent;

            const float nextAlpha = 2.0f * (float(i + 1) / float(axisSlices - 1) - 0.5f);
            const float nextZ = nextAlpha * extent;
        
            for (int a = 0; a < cylinderSlices; ++a) {
                const float angle = 2.0f * pif() * float(a) / float(cylinderSlices);
                const float nextAngle = 2.0f * pif() * float(a + 1) / float(cylinderSlices);

                const float x = cos(angle) * radius;
                const float y = sin(angle) * radius;
                const float nextX = cos(nextAngle) * radius;
                const float nextY = sin(nextAngle) * radius;

                const Color3 color = computeTunnelColor(alpha, angle);
                const Color3 nextColor = computeTunnelColor(nextAlpha, nextAngle);

                // Circle
                tunnelMesh.setColor(color);
                tunnelMesh.makeVertex(Point3(x, y, z));
                tunnelMesh.makeVertex(Point3(nextX, nextY, z));

                // Axis
                tunnelMesh.makeVertex(Point3(x, y, z));
                tunnelMesh.setColor(nextColor);
                tunnelMesh.makeVertex(Point3(x, y, nextZ));
            }
        }
        first = false;
    }
    tunnelMesh.render(rd);
    

    const static RealTime startTime = System::time();
    const float t = float(System::time() - startTime); 

    WireMesh::render(rd, m_posedMeshArray, m_posedCFrameArray);

    // Call to make the GApp show the output of debugDraw
    drawDebugShapes();
    
    rd->push2D(); {
        const float scale = rd->viewport().width() / 3840.0f;
        rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
        Draw::rect2D(reticleTexture->rect2DBounds() * scale + (rd->viewport().wh() - reticleTexture->vector2Bounds() * scale) / 2.0f, rd, Color3::white(), reticleTexture);

        // Faster than the full stats widget
        m_font->draw2D(rd, format("%d measured / %d requested fps", 
            iRound(renderDevice->stats().smoothFrameRate), 
            window()->settings().refreshRate), 
            (Point2(36, 24) * scale).floor(), floor(28.0f * scale), Color3::yellow());
    } rd->pop2D();
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D>>& surface2D) {
    Surface2D::sortAndRender(rd, surface2D);

}


G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    (void)argc; (void)argv;
    GApp::Settings settings(argc, argv);
    
    if (playMode) {
        settings.window.width       = 1920; settings.window.height      = 1080;
    } else {
        settings.window.width       = 1280; settings.window.height      = 720;
    }
    settings.window.fullScreen  = playMode;
    settings.window.resizable   = ! settings.window.fullScreen;
    settings.window.asynchronous = unlockFramerate;
    settings.window.caption = "Max Perf";
    settings.window.refreshRate = -1;
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0,0);
    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(0,0);

    return App(settings).run();
}
