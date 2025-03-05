/**
  \file tools/viewer/MD2Viewer.h

  G3D Innovation Engine https://casual-effects.com/g3d
  Copyright 2000-2020, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define MD2Viewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class MD2Viewer : public Viewer3D {
private:
    shared_ptr<MD2Model>               m_model;
    MD2Model::Pose                     m_pose;
    MD2Model::Pose::Action             m_action;
    CoordinateFrame                    m_cframe;

public:
    virtual void onInit(const String& filename, App* app) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) override;
    virtual bool onEvent(const GEvent& e, App* app) override;
};
