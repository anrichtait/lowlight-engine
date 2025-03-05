/**
  \file tools/viewer/PointViewer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "PointViewer.h"
#include "App.h"

// Useful for debugging material assignments
static const bool mergeMaterials = true;

PointViewer::PointViewer() {
}

PointViewer::~PointViewer() {
}


void PointViewer::onInit(const String& filename, App* app) {
    m_model = nullptr;
    m_filename = filename;

    const RealTime start = System::time();
    PointModel::Specification spec;
    spec.xyzOptions.autodetect = true;
    spec.filename = filename;
    spec.scale = m_scale;
    m_model = PointModel::create("Points", spec);
    // m_model->setPointRadius(0.1f);
}


void PointViewer::onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    m_model->pose(posed3D, m_offset, m_offset, nullptr, nullptr, nullptr, Surface::ExpressiveLightScatteringProperties());

    Viewer3D::onPose(posed3D, posed2D);
}
    

void PointViewer::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) {
    Viewer3D::onGraphics3D(rd, surfaceArray);
    screenPrintf("[Shown scaled by %f, auto-centered, and offset by (%f, %f, %f)]\n",
                 m_scale, m_offset.x, m_offset.y, m_offset.z);
    screenPrintf("%d points, %f visualization radius", m_model->numPoints(), m_model->pointRadius());

    const AABox& bounds = m_model->boxBounds();
    screenPrintf("Box bounds: (%f, %f, %f) to (%f, %f, %f)",
        bounds.low().x, bounds.low().y, bounds.low().z, 
        bounds.high().x, bounds.high().y, bounds.high().z);

    Draw::box(bounds, rd, Color4::clear());
}

