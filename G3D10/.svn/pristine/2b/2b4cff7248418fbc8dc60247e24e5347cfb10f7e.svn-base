/**
  \file tools/viewer/Viewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef Viewer_h
#define Viewer_h

#include "G3D/G3D.h"
#include "App.h"

class Viewer {
public:
    virtual ~Viewer() {}
    virtual void onInit(const G3D::String& filename, App* app) = 0;
    virtual bool onEvent(const GEvent& e, App* app) { return false; }
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {}
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) {
        rd->setColorClearValue(Color3::white());
        rd->clear(true, true, true);
    };
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) {};
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {};
};

class Viewer3D : public Viewer {
protected:
    class InstructionSurface : public Surface2D {
    private:
        shared_ptr<GFont>   m_font;
        shared_ptr<Texture> m_guide;

    protected:
        InstructionSurface(const shared_ptr<Texture>& guide, const shared_ptr<GFont>& font) : m_font(font), m_guide(guide) {};

    public:

        static shared_ptr<InstructionSurface> create(const shared_ptr<Texture>& guide, const shared_ptr<GFont>& font);
        virtual Rect2D bounds() const;
        virtual float depth() const;
        virtual void render(RenderDevice *rd) const;
    };

    String                           m_filename;
    shared_ptr<InstructionSurface>   m_instructions;
    shared_ptr<Surface>              m_skyboxSurface;

    Viewer3D(bool useSkyboxSurface = true);

public:

    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) override;
    virtual bool onEvent(const GEvent& e, App* app) override;
};

#endif 
