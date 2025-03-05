/**
  \file G3D-app.lib/source/VisualizeLightSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/VisualizeLightSurface.h"
#include "G3D-app/Light.h"
#include "G3D-app/Draw.h"
#include "G3D-gfx/RenderDevice.h"

namespace G3D {

VisualizeLightSurface::VisualizeLightSurface(const shared_ptr<Light>& c, bool showBounds) : 
    Surface(ExpressiveLightScatteringProperties(false, false)) ,
    m_showBounds(showBounds),
    m_light(c) {
}


shared_ptr<VisualizeLightSurface> VisualizeLightSurface::create(const shared_ptr<Light>& c, bool showBounds) {
    return createShared<VisualizeLightSurface>(c, showBounds);
}


String VisualizeLightSurface::name() const {
    return m_light->name();
}


void VisualizeLightSurface::getCoordinateFrame(CoordinateFrame& cframe, bool previous) const {
    if (previous) {
        cframe = m_light->previousFrame();
    } else {
        cframe = m_light->frame();
    }
}


void VisualizeLightSurface::getObjectSpaceBoundingBox(AABox& box, bool previous) const {
    if (m_showBounds) {
        box = AABox::inf();
    } else {
        const float r = m_light->extent().length() / 2.0f;
        box = AABox(Point3(-r, -r, -r), Point3(r, r, r));
    }
}


void VisualizeLightSurface::getObjectSpaceBoundingSphere(Sphere& sphere, bool previous) const {
    if (m_showBounds) {
        sphere = Sphere(Point3::zero(), m_light->effectSphere().radius);
    } else {
        sphere = Sphere(Point3::zero(), m_light->extent().length() / 2.0f);
    }
}


void VisualizeLightSurface::render
   (RenderDevice*                           rd, 
    const LightingEnvironment&              environment,
    RenderPassType                          passType) const {

    if (m_showBounds) {
        Draw::visualizeLightGeometry(m_light, rd, passType);
    } else {
        Draw::light(m_light, rd, passType);
    }
}


void VisualizeLightSurface::renderDepthOnlyHomogeneous
    (RenderDevice*                          rd, 
    const Array<shared_ptr<Surface> >&      surfaceArray,
    const shared_ptr<Texture>&              depthPeelTexture,
    const float                             depthPeelEpsilon,
    TransparencyTestMode                    transparencyTestMode,  
    const Color3&                           transmissionWeight) const {
} 


void VisualizeLightSurface::renderWireframeHomogeneous
   (RenderDevice*                           rd, 
    const Array<shared_ptr<Surface> >&      surfaceArray, 
    const Color4&                           color,
    bool                                    previous) const {
    // Intentionally do not render in wireframe; nobody ever wants to see
    // how many polygons are on a debug visualization, so the caller probably
    // would like to see the REST of the scene in wireframe and the Lights
    // superimposed.
}

bool VisualizeLightSurface::canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const {
    return false;
}

} // G3D
