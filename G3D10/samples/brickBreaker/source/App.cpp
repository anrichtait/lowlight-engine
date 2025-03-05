/** \file App.cpp */
#include "App.h"

App::App(const GApp::Settings& settings) : GApp(settings) {
}


void App::onInit() {
    GApp::onInit();
    setFrameDuration(1.0f / 60.0f);
    showRenderingStats = false;
    loadScene("scene/Level1.Scene.Any");
    makeGUI();

    // Example of changing the G-buffer specification:
    // m_gbufferSpecification.encoding[GBuffer::Field::TEXCOORD0] = ImageFormat::RG16F();
}


void App::onAfterLoadScene(const Any& sceneAny, const String& sceneName) {
    GApp::onAfterLoadScene(sceneAny, sceneName);
    spawnBricks();
    paddle = scene()->typedEntity<VisibleEntity>("paddle");
    ballArray.clear();
    ballArray.append(scene()->typedEntity<VisibleEntity>("ball"));
}


void App::spawnBricks() {

    Random& rng = Random::common();

    for (int y = 0, i = 0; y < 5; ++y) {
        for (int x = 0; x < 7; ++x, ++i) {
            
            String modelName = format("brick%dModel", rng.integer(1, 3));
            if (rng.uniform() < 0.10f) {
                modelName = "brickRubyModel";
            }

            const shared_ptr<VisibleEntity>& brick = VisibleEntity::create(format("brick_%02d", i), 
                 scene().get(), scene()->modelTable()[modelName].resolve(), 
                 CFrame::fromXYZYPRDegrees(1.5f * x, 0.7f * y + 4.0f, 0, 0, 0, rng.integer(0, 1) * 180.0f));

            brick->setShouldBeSaved(false);
            scene()->insert(brick);
        }
    }
}

void App::makeGUI() {
    debugWindow->setVisible(false);
    developerWindow->videoRecordDialog->setEnabled(true);    
    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}

void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    // Example GUI dynamic layout code.  Resize the debugWindow to fill
    // the screen horizontally.
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }

    // If you need to track individual UI events, manage them here.
    // Return true if you want to prevent other parts of the system
    // from observing this specific event.
    //
    // For example,
    // if ((event.type == GEventType::GUI_ACTION) && (event.gui.control == m_button)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { ... return true; }
    
    return false;
}


void App::onCleanup() {
}
