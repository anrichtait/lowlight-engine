/**
  \file tools/viewer/PointViewer.h

  G3D Innovation Engine https://casual-effects.com/g3d
  Copyright 2000-2022, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef PointViewer_h
#define PointViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class PointViewer : public Viewer3D {
private:

    String                                  m_filename;
    shared_ptr<PointModel>                  m_model;
    float                                   m_scale = 1.0;
    Point3                                  m_offset;

public:

    PointViewer();
    ~PointViewer();
    virtual void onInit(const String& filename, App* app) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;

};

#endif 
