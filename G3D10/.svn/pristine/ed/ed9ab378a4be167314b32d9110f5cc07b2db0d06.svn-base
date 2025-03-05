/**
  \file maxPerf/App.h

  Sample application showing how to render simple graphics with maximum throughput and 
  minimum latency by stripping away most high level VFX and convenience features for
  development. The result is a parallel minimal graphics system in which WireMesh
  replaces G3D::Model and G3D::Surface.
  
  Minimum latency is the harder part. Even with high-level objects, G3D
  is able to render at 1000 fps or faster, but the optimizations taken by Surface
  and RenderDevice in those cases incur latency.
  
  This approach is good for some display and perception research. For general
  game and rendering applications, look at the G3D starter app and vrStarter which give very
  performance with a lot of high-level game engine features.

 */
#pragma once
#include <G3D/G3D.h>

class WireMesh;

class App : public GApp {
protected:

    class Target {
    public:
        float                       hitRadius = 1.0f;

        CFrame                      cframe;


        /** Transform cframe by this every frame. It is linear and
            angular velocity in object space per frame (not per second) */
        CFrame                      velocity;

        Target() {}
        Target(const CFrame& f, const CFrame& v) : cframe(f), velocity(v) {}
    };

    /** Renders a solid color with a slight offset to the camera stored in the texture coordinate. */
    shared_ptr<Shader>              m_solidOffsetShader;
    Array<Target>                   m_targetArray;

    /** Written by onPose */
    Array<shared_ptr<WireMesh>>     m_posedMeshArray;
    /** Written by onPose */
    Array<CFrame>                   m_posedCFrameArray;

    shared_ptr<WireMesh>            m_targetMesh;
    shared_ptr<WireMesh>            m_worldMesh;
    shared_ptr<GFont>               m_font;

public:
    
    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onPose(Array<shared_ptr<Surface>>& surface, Array<shared_ptr<Surface2D>>& surface2D) override;
    virtual void onGraphics3D(RenderDevice* rd, Array< shared_ptr<Surface> >& surface) override;
    virtual void onGraphics2D(RenderDevice* rd, Array< shared_ptr<Surface2D> >& surface2D) override;
    virtual bool onEvent(const GEvent& e) override;
};
