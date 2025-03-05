#pragma once
#include <G3D/G3D.h>

class App : public GApp {
protected:
        
    shared_ptr<Sound>       m_backgroundMusic;    

    void spawnAsteroids();

    /** Called from onInit */
    void makeGUI();

    virtual void onPostProcessHDR3DEffects(RenderDevice* rd) override;

public:
    
    App(const GApp::Settings& settings = GApp::Settings());
    virtual void onInit() override;
    virtual void onUserInput(UserInput* ui) override;

    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
};
