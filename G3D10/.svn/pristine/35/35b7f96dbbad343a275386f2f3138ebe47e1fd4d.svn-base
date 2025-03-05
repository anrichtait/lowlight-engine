/**
  \file tools/viewer/SceneViewer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2023, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "SceneViewer.h"
#include "App.h"

void SceneViewer::onInit(const String& filename, App* app) {
    m_filename = filename; 
    app->loadScene(filename);
    m_scene = app->scene();
    //app->developerWindow->setVisible(true);
    app->developerWindow->cameraControlWindow->setVisible(true);
    app->developerWindow->sceneEditorWindow->setVisible(true);
    app->activeCamera()->filmSettings().setTemporalAntialiasingEnabled(true);
    //app->debugWindow->setVisible(false);
}

void SceneViewer::onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    Viewer3D::onPose(posed3D, posed2D);
    if (m_scene) {
        m_scene->onPose(posed3D);
    }
}