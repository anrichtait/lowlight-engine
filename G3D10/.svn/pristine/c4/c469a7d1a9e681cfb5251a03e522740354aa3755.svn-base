/**
  \file G3D-app.lib/source/VisualizeCameraSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/VisualizeCameraSurface.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Draw.h"
#include "G3D-app/Light.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/LightingEnvironment.h"

namespace G3D {

VisualizeCameraSurface::VisualizeCameraSurface(const shared_ptr<Camera>& c) : m_camera(c) {}


shared_ptr<VisualizeCameraSurface> VisualizeCameraSurface::create(const shared_ptr<Camera>& c) {
    return shared_ptr<VisualizeCameraSurface>(new VisualizeCameraSurface(c));
}


String VisualizeCameraSurface::name() const {
    return m_camera->name();
}


void VisualizeCameraSurface::getCoordinateFrame(CoordinateFrame& cframe, bool previous) const {
    if (previous) {
        cframe = m_camera->previousFrame();
    } else {
        cframe = m_camera->frame();
    }
}


void VisualizeCameraSurface::getObjectSpaceBoundingBox(AABox& box, bool previous) const {
    box = AABox(Point3(-0.2f, -0.2f, -0.2f), Point3(0.2f, 0.2f, 0.2f));
}


void VisualizeCameraSurface::getObjectSpaceBoundingSphere(Sphere& sphere, bool previous) const {
    sphere = Sphere(Point3::zero(), 0.2f);
}


void VisualizeCameraSurface::render
   (RenderDevice*                           rd, 
    const LightingEnvironment&              environment,
    RenderPassType                          passType) const {

    Draw::camera(m_camera, rd);

}


void VisualizeCameraSurface::renderDepthOnlyHomogeneous
    (RenderDevice*                          rd, 
    const Array<shared_ptr<Surface> >&      surfaceArray,
    const shared_ptr<Texture>&              depthPeelTexture,
    const float                             depthPeelEpsilon,
    TransparencyTestMode                    transparencyTestMode,
    const Color3&                           transmissionWeight) const {
    Draw::camera(m_camera, rd);
} 


void VisualizeCameraSurface::renderWireframeHomogeneous
   (RenderDevice*                           rd, 
    const Array<shared_ptr<Surface> >&      surfaceArray, 
    const Color4&                           color,
    bool                                    previous) const {
    // Intentionally do not render in wireframe; nobody ever wants to see
    // how many polygons are on a debug visualization, so the caller probably
    // would like to see the REST of the scene in wireframe and the cameras
    // superimposed.
}

bool VisualizeCameraSurface::canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const {
    return false;
}

} // G3D
