/** \file App.cpp */
#include "App.h"

static bool runAsServer = true;

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    G3DSpecification spec;
    
    const char* choices[] = {"Server", "Client", "Quit"};
    switch (prompt("Video Stream", "Network role of this instance", choices, 3)) {
    case 0: runAsServer = true; spec.logFilename = "server-log.txt"; break;
    case 1: runAsServer = false; spec.logFilename = "client-log.txt"; break;      
    default: return 0; break;
    }

    initGLG3D(spec);
    GApp::Settings settings(argc, argv);
    settings.window.caption             = argv[0];
    settings.window.width            =  854; settings.window.height       = 480;
    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = true;
    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {}


void App::onInit() {
    GApp::onInit();
    if (runAsServer) {
        // Server
        m_netServer = NetServer::create(CONNECT_PORT);
        m_videoServer = VideoStreamServer::create();
    } else {
        // Client
        // TODO: Allow entering a explicit IP address. Right now we assume the server is on the same machine
        logPrintf("Connecting to server...\n");
        m_videoClient = VideoStreamClient::create(NetAddress(NetAddress::localHostname(), CONNECT_PORT));
        logPrintf("Connected...\n");
    }
 
    setLowerFrameRateInBackground(false);
    makeGUI();

    if (m_videoServer) {
        loadScene(
    #       ifndef G3D_DEBUG
                "G3D Simple Cornell Box (Area Light)" // Load something simple
    //            "G3D Sponza"
    #       else
                "G3D Simple Cornell Box (Area Light)" // Load something simple
    #       endif
            //developerWindow->sceneEditorWindow->selectedSceneName()  // Load the first scene encountered 
            );
    }
}


void App::makeGUI() {
    debugWindow->setVisible(false);
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->videoRecordDialog->setEnabled(true);
    showRenderingStats = true;
}

void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    if (m_videoClient) {
        rd->swapBuffers();
        rd->clear();

        // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
        m_film->exposeAndRender(rd, activeCamera()->filmSettings(), notNull(m_streamedTexture) ? m_streamedTexture : Texture::opaqueBlack(),
            settings().hdrFramebuffer.trimBandThickness().x,
            settings().hdrFramebuffer.depthGuardBandThickness.x,
            Texture::opaqueBlack(),
            Vector2::zero());
        return;
    }

    // Server:
    GApp::onGraphics3D(rd, allSurfaces);
}

void App::onNetwork() {
    GApp::onNetwork();

    if (m_videoServer) {
        // Get new connections
        for (NetConnectionIterator& it = m_netServer->newConnectionIterator(); it.isValid(); ++it) {
            logPrintf("Client connected\n");
            m_videoServer->addClient(it.connection());
        }

        screenPrintf("Num clients: %d", m_videoServer->clientConnectionArray().size());
        BEGIN_PROFILER_EVENT("Video Send");
        m_videoServer->send(m_framebuffer->texture(0));
        END_PROFILER_EVENT();
    } else if (m_videoClient) {
        BEGIN_PROFILER_EVENT("Video Receive");
        m_streamedTexture = m_videoClient->receive();
        END_PROFILER_EVENT();
    }
}

void App::onCleanup() {
    // Called after the application loop ends.  Place a majority of cleanup code
    // here instead of in the destructor so that exceptions can be caught.
    if (m_videoServer) {
        m_videoServer = nullptr;
    } else if (m_videoClient) {
        m_videoClient->serverConnection()->disconnect();
        m_videoClient = nullptr;
    }
}
