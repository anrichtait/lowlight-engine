#include "App.h"
#include "PlayerEntity.h"

static const bool playMusic = true;


App::App(const GApp::Settings& settings) : GApp(settings) {}


void App::onInit() {
    GApp::onInit();
    showRenderingStats = false;

    try {
        if (playMusic) {
            const String musicFile = System::findDataFile("music/cdk_-_Saturdays_Basement.mp3", true);
            m_backgroundMusic = Sound::create(musicFile, true);
            m_backgroundMusic->play();
        }
    } catch (...) {
        msgBox("This sample requires the 'game' asset pack to be installed in order to play the sound files", "Assets Missing");
    }

    setFrameDuration(1.0f / 30.0f);
    
    // Allowing custom Entity subclasses to be parsed from .Scene.Any files
    scene()->registerEntitySubclass("PlayerEntity", &PlayerEntity::create);

    makeGUI();

    loadScene(System::findDataFile("space.Scene.Any"));
    setActiveListener(scene()->entity("player"));

    // Enforce correct simulation order by placing constraints on objects
    scene()->setOrder("player", "camera");
    spawnAsteroids();
}


void App::makeGUI() {
    debugWindow->setVisible(false);
    developerWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));

    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));
}


void App::spawnAsteroids() {
    Random r(1023, false);
    static const int NUM_ASTEROIDS = 
#       ifdef G3D_DEBUG
               30;
#       else
               300;
#       endif

    for (int i = 0; i < NUM_ASTEROIDS; ++i) {
        const String& modelName = format("asteroid%dModel", r.integer(0, 4));

        const Point3 pos(r.uniform(-80.0f, 80.0f), r.uniform(-30.0f, 30.0f), r.uniform(-200.0f, 10.0f));

        const shared_ptr<VisibleEntity>& v = 
            VisibleEntity::create
            (format("asteroid%02d", i), 
             scene().get(),
             scene()->modelTable()[modelName].resolve(), CFrame());

        {
            // Construct the Entity::Track for motion
            const Any& a = Any::parse(format(STR(
                transform(
                    timeShift(
                        PhysicsFrameSpline{
                            control = [
                                CFrame::fromXYZYPRRadians(%f, %f, -300, 0, %f, %f),
                                CFrame::fromXYZYPRRadians(%f, %f, 10, 0, %f, %f)
                            ];

                            time = [
                                0,
                                15
                            ];

                            extrapolationMode = CYCLIC;
                            interpolationMode = LINEAR;
                            finalInterval = 0;
                        },
                        %f
                     ),
                orbit(0, %f))),

                // CFrame 1
                pos.x, pos.y,
                r.uniform(0, (float)twoPi()), r.uniform(0, (float)twoPi()),

                // CFrame 2
                pos.x, pos.y, 
                r.uniform(0, (float)twoPi()), r.uniform(0, (float)twoPi()),

                // Time shift
                r.uniform(0, 15),

                // Tumble rate
                r.uniform(3, 60))); 
            const shared_ptr<Entity::Track>& track = Entity::Track::create(v.get(), scene().get(), a);
            v->setTrack(track);
        }

        // Don't serialize generated objects
        v->setShouldBeSaved(false);

        scene()->insert(v);
    }
}


void App::onUserInput(UserInput* ui) {
    GApp::onUserInput(ui);

    if (! m_debugController->enabled()) {
        const shared_ptr<PlayerEntity>& player = scene()->typedEntity<PlayerEntity>("player");
        if (notNull(player)) {
            player->setDesiredOSVelocity(Vector3(ui->getX() * 100, -ui->getY() * 100, 0.0f));
        }
    }
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    // Update the follow-camera (this logic could be placed on the camera if it were
    // a subclass of Camera)
    const shared_ptr<Entity>& camera = scene()->entity("camera");
    const shared_ptr<Entity>& player = scene()->entity("player");
    if (notNull(camera) && notNull(player)) {
        const CFrame& playerFrame = player->frame();
        CFrame cameraFrame;
        cameraFrame.translation.x = playerFrame.translation.x / 2;
        cameraFrame.translation.y = playerFrame.translation.y / 2 + 2.0f;
        cameraFrame.translation.z = playerFrame.translation.z + 14.0f;

        float yaw, pitch, roll;
        playerFrame.rotation.toEulerAnglesXYZ(yaw, pitch, roll);
        cameraFrame.rotation = Matrix3::fromAxisAngle(Vector3::unitX(), -0.15f) * Matrix3::fromAxisAngle(Vector3::unitZ(), roll / 5);
        camera->setPreviousFrame(camera->frame());
        camera->setFrame(cameraFrame);
    }
}


void App::onPostProcessHDR3DEffects(RenderDevice* rd) {
    // Render fog
    rd->push2D(m_framebuffer); {
        Args args;
        args.setUniform("depth", m_framebuffer->texture(Framebuffer::DEPTH), Sampler::buffer());
        args.setRect(rd->viewport());
        rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
        LAUNCH_SHADER("fog.pix", args);
    } rd->pop2D();

    GApp::onPostProcessHDR3DEffects(rd);
}


G3D_START_AT_MAIN();
int main(int argc, const char* argv[]) {
    G3D::G3DSpecification spec;
    spec.audio = true;
    initGLG3D(spec);

    GApp::Settings settings(argc, argv);
    
    settings.window.caption     = "G3D Entity Sample";
    settings.window.width       = 1280; 
    settings.window.height      = 720;
    try {
        settings.window.defaultIconFilename = System::findDataFile("icon/rocket/icon.png");
    } catch (...) {
        debugPrintf("Could not find icon\n");
        // Ignore exception if we can't find the icon
    }

    return App(settings).run();
}
