/**
  \file tools/viewer/SceneViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef SceneViewer_h
#define SceneViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class SceneViewer : public Viewer3D {
private:

    shared_ptr<Scene> m_scene;

public:

    SceneViewer():Viewer3D(false) {};
    ~SceneViewer() {};

    virtual void onInit(const String& filename, App* app) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;
};

#endif 
