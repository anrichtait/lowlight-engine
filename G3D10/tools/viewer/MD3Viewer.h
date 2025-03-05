/**
  \file tools/viewer/MD3Viewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef MD3Viewer_h
#define MD3Viewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class MD3Viewer : public Viewer3D {
private:
    shared_ptr<MD3Model>               m_model;
    MD3Model::Pose                     m_pose;
    CoordinateFrame                    m_cframe;

public:

    virtual void onInit(const String& filename, App* app) override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;
};

#endif 
