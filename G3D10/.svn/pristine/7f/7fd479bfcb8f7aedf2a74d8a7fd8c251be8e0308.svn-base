/**
  \file G3D-app.lib/source/SkyboxSurface.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/CoordinateFrame.h"
#include "G3D-app/Draw.h"
#include "G3D-base/Vector4.h"
#include "G3D-base/Array.h"
#include "G3D-base/AABox.h"
#include "G3D-base/Projection.h"
#include "G3D-base/fileutils.h"
#include "G3D-gfx/VertexBuffer.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/SkyboxSurface.h"
#include "G3D-app/Light.h"
#include "G3D-app/Camera.h"

namespace G3D {

AttributeArray      SkyboxSurface::s_cubeVertices;
IndexStream         SkyboxSurface::s_cubeIndices;

SkyboxSurface::SkyboxSurface   
   (const shared_ptr<Texture>& c0,
    const shared_ptr<Texture>& c1,
    float                alpha) :
    Surface(ExpressiveLightScatteringProperties(false, false)),
    m_alpha(alpha),
    m_texture0(c0),
    m_texture1(c1) {

    debugAssert((alpha >= 0.0f) && (alpha <= 1.0f));
}     


shared_ptr<SkyboxSurface> SkyboxSurface::create
   (const shared_ptr<Texture>&    c0,
    const shared_ptr<Texture>&    c1,
    float                         alpha) {

    debugAssert(c0);
    debugAssert(c0->dimension() == Texture::DIM_CUBE_MAP || c0->dimension() == Texture::DIM_2D);

    if (! s_cubeVertices.valid()) {
        // Size of the box (doesn't matter because we're rendering at infinity)
        const float s = 1.0f;

        Array<Vector4> vert;
        vert.append(Vector4(-s, +s, -s, 0.0f));
        vert.append(Vector4(-s, -s, -s, 0.0f));
        vert.append(Vector4(+s, -s, -s, 0.0f));
        vert.append(Vector4(+s, +s, -s, 0.0f));
    
        vert.append(Vector4(-s, +s, +s, 0.0f));
        vert.append(Vector4(-s, -s, +s, 0.0f));
        vert.append(Vector4(-s, -s, -s, 0.0f));
        vert.append(Vector4(-s, +s, -s, 0.0f));
    
        vert.append(Vector4(+s, +s, +s, 0.0f));
        vert.append(Vector4(+s, -s, +s, 0.0f));
        vert.append(Vector4(-s, -s, +s, 0.0f));
        vert.append(Vector4(-s, +s, +s, 0.0f));

        vert.append(Vector4(+s, +s, +s, 0.0f));
        vert.append(Vector4(+s, +s, -s, 0.0f));
        vert.append(Vector4(+s, -s, -s, 0.0f));
        vert.append(Vector4(+s, -s, +s, 0.0f));
    
        vert.append(Vector4(+s, +s, +s, 0.0f));
        vert.append(Vector4(-s, +s, +s, 0.0f));
        vert.append(Vector4(-s, +s, -s, 0.0f));
        vert.append(Vector4(+s, +s, -s, 0.0f));
    
        vert.append(Vector4(+s, -s, -s, 0.0f));
        vert.append(Vector4(-s, -s, -s, 0.0f));
        vert.append(Vector4(-s, -s, +s, 0.0f));
        vert.append(Vector4(+s, -s, +s, 0.0f));

        Array<int> indices;
        for (int i = 0; i < 6; ++i) {
            int off = i*4;
            indices.append(0 + off, 1 + off, 2 + off);
            indices.append(0 + off, 2 + off, 3 + off);
        }

        const shared_ptr<VertexBuffer>& vb = VertexBuffer::create(vert.size() * sizeof(Vector4), VertexBuffer::WRITE_ONCE);
        s_cubeVertices = AttributeArray(vert, vb);
        const shared_ptr<VertexBuffer>& indexVB = VertexBuffer::create(indices.size() * sizeof(int), VertexBuffer::WRITE_ONCE);
        s_cubeIndices = IndexStream(indices, indexVB);
    }

    return createShared<SkyboxSurface>(c0, c1, alpha);
}


String SkyboxSurface::name() const {
    return m_texture0->name() + " Skybox";
}


void SkyboxSurface::getCoordinateFrame(CoordinateFrame& cframe, bool previous) const {
    cframe = CFrame();
}


void SkyboxSurface::getObjectSpaceBoundingBox(AABox& box, bool previous) const {
    box = AABox::inf();
}


void SkyboxSurface::getObjectSpaceBoundingSphere(Sphere& sphere, bool previous) const {
    sphere.radius = (float)inf();
    sphere.center = Point3::zero();
}


bool SkyboxSurface::canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const {
    return notNull(specification.encoding[GBuffer::Field::EMISSIVE].format) &&
            (specification.encoding[GBuffer::Field::EMISSIVE].format->redBits > 8);
}


void SkyboxSurface::render
(RenderDevice*                          rd, 
 const LightingEnvironment&             environment,
 RenderPassType                         passType) const {
    rd->pushState();
    Args args;

    setShaderArgs(args, "");
    setShaderGeometryArgs(rd, args);
    rd->setDepthWrite(false);
    glEnable(GL_DEPTH_CLAMP);
    LAUNCH_SHADER_WITH_HINT("SkyboxSurface_render.*", args, name());
    glDisable(GL_DEPTH_CLAMP);

    rd->popState();
}


void SkyboxSurface::setShaderArgs(Args& args, const String& prefix) const {
    debugAssert(m_alpha >= 0 && m_alpha <= 1);
    args.setUniform(prefix + "alpha", m_alpha);

    if (m_alpha < 1) {
        if (m_texture0->dimension() == Texture::DIM_2D) {
            args.setMacro(prefix + "texture0_2DSpherical", true);
            m_texture0->setShaderArgs(args, prefix + "texture0_", Sampler(WrapMode::TILE, WrapMode::CLAMP));
        } else {
            m_texture0->setShaderArgs(args, prefix + "texture0_", Sampler::defaults());
        }
    }

    if (m_alpha > 0) {
        if (m_texture1->dimension() == Texture::DIM_2D) {
            args.setMacro(prefix + "texture1_2DSpherical", true);
            m_texture1->setShaderArgs(args, prefix + "texture1_", Sampler(WrapMode::TILE, WrapMode::CLAMP));
        } else {
            m_texture1->setShaderArgs(args, prefix + "texture1_", Sampler::defaults());
        }
    }
}
   

void SkyboxSurface::renderIntoGBufferHomogeneous
(RenderDevice*                          rd,
 const Array<shared_ptr<Surface> >&     surfaceArray,
 const shared_ptr<GBuffer>&             gbuffer,
 const shared_ptr<Texture>&             depthPeelTexture,
 const float                            minZSeparation,
 const LightingEnvironment&             lightingEnvironment) const {

    if (surfaceArray.length() == 0) {
        return;
    }

    // There's no point in drawing more than one skybox, since they would overlap.
    // Therefore, draw the last one.
    const shared_ptr<SkyboxSurface>& skybox = dynamic_pointer_cast<SkyboxSurface>(surfaceArray.last());
    alwaysAssertM(skybox, "non-SkyboxSurface passed to SkyboxSurface::renderHomogeneous");

    glEnable(GL_DEPTH_CLAMP);
    rd->pushState(); {
        Args args;
        const Rect2D& colorRect = gbuffer->colorRect();
        rd->setClip2D(colorRect);

        args.setUniform("lowerCoord", colorRect.x0y0());
        args.setUniform("upperCoord", colorRect.x1y1());

        CFrame dummy;
        gbuffer->setShaderArgsWritePosition(dummy, rd, args);
        args.setUniform("projInfo", gbuffer->camera()->projection().reconstructFromDepthProjInfo(gbuffer->width(), gbuffer->height()), true);
       
        if (notNull(gbuffer->specification().encoding[GBuffer::Field::EMISSIVE].format)) {
            setShaderArgs(args, "");
        }

        // We could just run a 2D full-screen pass here, but we want to take advantage of the depth buffer
        // to early-out on pixels where the skybox is not visible
        setShaderGeometryArgs(rd, args);
        args.setMacro("HAS_ALPHA", 0);

        // There's no reason to write depth from a skybox shader, since
        // the skybox is at infinity
        rd->setDepthWrite(false);
        LAUNCH_SHADER_WITH_HINT("SkyboxSurface_gbuffer.*", args, name());
    
    } rd->popState();
    glDisable(GL_DEPTH_CLAMP);
}


void SkyboxSurface::setShaderGeometryArgs(RenderDevice* rd, Args& args) {
    rd->setObjectToWorldMatrix(CFrame());
    /*
    // Make a camera with an infinite view frustum to avoid clipping (this fails in VR with asymmetric projection matrices!)
    Matrix4 P = rd->projectionMatrix();
    Projection projection(P, rd->viewport().wh());
    projection.setFarPlaneZ(-finf());
    projection.getProjectUnitMatrix(rd->viewport(), P);
    rd->setProjectionMatrix(P);
    */
    rd->setCameraToWorldMatrix(rd->cameraToWorldMatrix().rotation);

    args.setAttributeArray("g3d_Vertex", s_cubeVertices);
    args.setIndexStream(s_cubeIndices);
    args.setPrimitiveType(PrimitiveType::TRIANGLES);
 }


void SkyboxSurface::renderWireframeHomogeneous
(RenderDevice*                rd, 
 const Array<shared_ptr<Surface> >&   surfaceArray, 
 const Color4&                color,
 bool                         previous) const {
     // Intentionally empty
}


void SkyboxSurface::renderDepthOnlyHomogeneous
(RenderDevice*                      rd, 
 const Array<shared_ptr<Surface> >& surfaceArray,
 const shared_ptr<Texture>&         depthPeelTexture,
 const float                        depthPeelEpsilon,
 TransparencyTestMode               transparencyTestMode,  
 const Color3&                      transmissionWeight) const {
    // Intentionally empty; nothing to do since the skybox is at infinity and
    // would just fail the depth test or set it back to infinity
}

} // namespace G3D
