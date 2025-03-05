/** \file MatchMaker.cpp */
/**
    Sample Matchmaking service using ServiceDiscovery
 */
#pragma once
#include <G3D/G3D.h>

class MatchMaker : public GApp {

    String                                  m_name;

    /** Service Discovery Class */
    shared_ptr<Discovery::ServiceDiscovery>         m_serviceDiscovery;

    // who owns the net server?
    /** Net Server */
    shared_ptr<NetServer>         m_net;

public:

    MatchMaker(const GApp::Settings& settings) : GApp(settings) {
        renderDevice->setColorClearValue(Color3::white());
    }

    void onInit() override {
        GApp::onInit();
        debugWindow->setVisible(false);
        developerWindow->setVisible(false);
        developerWindow->cameraControlWindow->setVisible(false);
        developerWindow->sceneEditorWindow->setVisible(false);
        showRenderingStats = false;

        NetAddress addr (NetAddress::DEFAULT_ADAPTER_HOST, Discovery::DEFAULT_SERVICE_DISCOVERY_PORT);
        m_net = NetServer::create(addr);
        logPrintf("Matchmaker: Service Discovery at %s:%d\n", addr.hostname().c_str(), addr.port());
        m_serviceDiscovery = Discovery::ServiceDiscovery::create(m_net);

        window()->setCaption(" (" + addr.hostname() + ":" + format("%d", addr.port()) + ")");

        addWidget(m_serviceDiscovery);
    }

    bool onEvent(const GEvent& e) override {
        // should not handle any event other than quit
        if (e.type == GEventType::QUIT) {
            // Let the connection close on the thread before shutting down
            System::sleep(2);
            exit(0); 
            return false;
        }
        else
            return true;

    } // onEvent
};


// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    (void)argc; (void)argv;
    GApp::Settings settings(argc, argv);

    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.width       = 1280; 
    settings.window.height      = 720;

    return MatchMaker(settings).run();
}


