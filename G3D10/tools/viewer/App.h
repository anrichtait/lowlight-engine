/**
  \file tools/viewer/App.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef APP_H
#define APP_H

#include "G3D/G3D.h"

class Viewer;

class App : public GApp {
private:
    shared_ptr<LightingEnvironment>     lighting;
    Viewer*	                            viewer;
    String	                            filename;
    
public:
    /** Used by GUIViewer */
    Color4						        colorClear;
    bool                                showInstructions = true;
    
    const shared_ptr<GBuffer>& gbuffer() const {
        return m_gbuffer;
    }

    const shared_ptr<Framebuffer>& framebuffer() const {
        return m_framebuffer;
    }

    const shared_ptr<Framebuffer>& depthPeelFramebuffer() const {
        return m_depthPeelFramebuffer;
    }

    App(const GApp::Settings& settings = GApp::Settings(), const G3D::String& file = "");
    
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onInit() override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) override;
    virtual void onGraphics2D(RenderDevice *rd, Array< shared_ptr<Surface2D> > &surface2D) override;
    virtual void onCleanup() override;
    virtual bool onEvent(const GEvent& event) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;
    
    shared_ptr<AmbientOcclusion> ambientOcclusion() { return m_ambientOcclusion; }

private:
    /** Called from onInit() and after a FILE_DROP in onEvent()*/
    void setViewer(const G3D::String& newFilename);
};

#endif
