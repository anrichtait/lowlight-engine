/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3D.h>

/** \brief Application framework. */
class App : public GApp {
protected:

    static const uint16             CONNECT_PORT = 8080;

    /** Non-null if running the server */
    shared_ptr<NetServer>           m_netServer;
    /** Non-null if running the server */
    shared_ptr<VideoStreamServer>   m_videoServer;

    /** Non-null if running the client */
    shared_ptr<VideoStreamClient>   m_videoClient;

    /** Non-null if running on the client */
    shared_ptr<Texture>             m_streamedTexture;

    /** Called from onInit */
    void makeGUI();

public:
    
    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;
    virtual void onNetwork() override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
    virtual void onCleanup() override;
};
