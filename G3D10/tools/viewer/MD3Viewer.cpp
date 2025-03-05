/**
  \file tools/viewer/MD3Viewer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "MD3Viewer.h"

//static const float START_Y = 6.0f;
//static const float START_Z = 13.0f;
//static const float START_YAW = -90.0f;


void MD3Viewer::onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    Viewer3D::onPose(posed3D, posed2D);
    m_model->pose(posed3D, m_cframe, m_pose);
}

void MD3Viewer::onInit(const G3D::String& filename, App* app) {
    m_filename = filename;
    //m_cframe = CFrame::fromXYZYPRDegrees(0, START_Y, START_Z, START_YAW);
    m_cframe = CFrame::fromXYZYPRDegrees(0, 0, 0, 180);
	MD3Model::Specification spec;
    size_t pos = filename.find_last_of("\\");
    G3D::String dir = filename.substr(0, pos);
    spec.directory = dir;
    spec.defaultSkin = MD3Model::Skin::create(dir, "default");
    m_model = MD3Model::create(spec);
}

void MD3Viewer::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    m_model->simulatePose(m_pose, sdt);
}
