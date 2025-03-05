#ifndef App_h
#define App_h

#include <G3D/G3D.h>

class App : public GApp {
protected:
    shared_ptr<Texture> causticTexture;
public:

    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit();
    virtual void onGraphics3D(RenderDevice* rd, Array< shared_ptr<Surface> >& surface);
};

#endif