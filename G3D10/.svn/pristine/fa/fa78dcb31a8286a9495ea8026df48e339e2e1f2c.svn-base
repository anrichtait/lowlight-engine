/**
  \file tools/viewer/MD2Viewer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "MD2Viewer.h"

void MD2Viewer::onInit(const String& filename, App* app) {
    m_model = MD2Model::create(filename);
    m_pose = MD2Model::Pose(MD2Model::STAND, 0);
    m_cframe = CFrame::fromXYZYPRDegrees(0, 0, 0, 180);
    m_filename = filename; 
}


void MD2Viewer::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    m_pose.onSimulation(sdt, m_action);
}


void MD2Viewer::onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    Viewer3D::onPose(posed3D, posed2D);

    m_model->pose(posed3D, m_cframe, m_cframe, nullptr, &m_pose, &m_pose, Surface::ExpressiveLightScatteringProperties());
}


void MD2Viewer::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) {
    Viewer3D::onGraphics3D(rd, surfaceArray);

    screenPrintf("Triangles: %d", m_model->numTriangles());
    screenPrintf("Animation: %d (number keys to change)", m_pose.animation);
    screenPrintf("1 - Attack  2 - Crouching  3 - Moving Forward   4 - Point   5 - Flip   6 - Fallback   7 - Death1   8 - Salute   9 - Wave   0 - Pain1", m_pose.animation);

}

bool MD2Viewer::onEvent(const GEvent& e, App* app) {
    if (Viewer3D::onEvent(e, app)) {
        // For refreshing the file
        return true;
    }

    if (GEventType::KEY_DOWN) {

        // zero out m_action
        System::memset(&m_action, 0, sizeof(G3D::MD2Model::Pose::Action));

        switch (e.key.keysym.sym) {
        case '1':
            return (m_action.attack = true);
        case '2':
            return (m_action.crouching = true);
        case '3':
            return (m_action.movingForward = true);
        case '4':
            return (m_action.point = true);
        case '5':
            return (m_action.fallback = true);
        case '6':
            return (m_action.death1 = true);
        case '7':
            return (m_action.salute = true);
        case '8':
            return (m_action.wave = true);
        case '9':
            return (m_action.pain1 = true);
        case '0':
            return (m_action.attack = true);
        }
    }
    return false;
}