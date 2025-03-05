#pragma once
#include <G3D/G3D.h>

/** \brief Application framework. */
class App : public GApp {
protected:

    shared_ptr<VisibleEntity>           paddle;
    Array<shared_ptr<VisibleEntity>>    ballArray;

    /** Called from onInit */
    void makeGUI();

    void spawnBricks();

public:

    
    App(const GApp::Settings& settings = GApp::Settings());
    virtual void onAfterLoadScene(const Any& sceneAny, const String& sceneName) override;
    virtual void onInit() override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual bool onEvent(const GEvent& e) override;
    virtual void onCleanup() override;
};
