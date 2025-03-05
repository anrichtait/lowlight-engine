/**
  \file G3D-app.lib/source/SlowMesh.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Shader.h"
#include "G3D-app/SlowMesh.h"
#include "G3D-gfx/GLCaps.h"

namespace G3D {

/** Overrides the current PrimitiveType, all of the create vertices will be of 
    said type, whether made before or after this call */
void SlowMesh::setPrimitveType(const PrimitiveType& p) {
    m_primitiveType = p;
}

void G3D::SlowMesh::setPointSize(const float size) {
    m_pointSize = size;
}

/** Sets the texture to use for rendering */
void SlowMesh::setTexture(const shared_ptr<Texture> t) {
    m_texture = t;
}

/** Change the currently set texCoord state, defaulted to (0,0) */
void SlowMesh::setTexCoord(const Vector2& texCoord){
    m_currentTexCoord = texCoord;
}

/** Change the currently set color state, defaulted to black */
void SlowMesh::setColor(const Color3& color) {
    m_currentColor = color;
}

void SlowMesh::setColor(const Color4& color) {
    m_currentColor = color;
}

/** Change the currently set color state, defaulted to (0,0,1) */
void SlowMesh::setNormal(const Vector3& normal) {
    m_currentNormal = normal;
}

/** Construct a CPUVertex given the current texCoord, color, and normal state */
void SlowMesh::makeVertex(const Vector2& vertex) {
    makeVertex(Vector4(vertex, 0, 1));
}

void SlowMesh::makeVertex(const Vector3& vertex) {
    makeVertex(Vector4(vertex, 1));
}

void SlowMesh::makeVertex(const Vector4& vertex) {
    Vertex& v = m_cpuVertexArray.next();
    v.position  = vertex;
    v.normal    = m_currentNormal;
    v.texCoord  = m_currentTexCoord;
    v.color     = m_currentColor;
}


void SlowMesh::copyToGPU(
        AttributeArray&               vertex, 
        AttributeArray&               normal, 
        AttributeArray&               texCoord0,
        AttributeArray&               vertexColors) const {
    #   define OFFSET(field) ((size_t)(&dummy.field) - (size_t)&dummy)

    const int numVertices = m_cpuVertexArray.size();
    if (numVertices > 0) {
        int cpuVertexByteSize = sizeof(Vertex) * numVertices;

        const int padding = 16;

        shared_ptr<VertexBuffer> buffer = VertexBuffer::create(cpuVertexByteSize + padding, VertexBuffer::WRITE_ONCE);
        
        AttributeArray all(cpuVertexByteSize, buffer);

        Vertex dummy;
        vertex          = AttributeArray(dummy.position,    numVertices, all, OFFSET(position), (int)sizeof(Vertex));
        normal          = AttributeArray(dummy.normal,      numVertices, all, OFFSET(normal),   (int)sizeof(Vertex));
        texCoord0       = AttributeArray(dummy.texCoord,    numVertices, all, OFFSET(texCoord), (int)sizeof(Vertex));
        vertexColors    = AttributeArray(dummy.color,       numVertices, all, OFFSET(color),    (int)sizeof(Vertex));

        // Copy all interleaved data at once
        Vertex* dst = (Vertex*)all.mapBuffer(GL_WRITE_ONLY);

        System::memcpy(dst, m_cpuVertexArray.getCArray(), cpuVertexByteSize);

        all.unmapBuffer();
        dst = nullptr;
    }

    #undef OFFSET
}


/** Constructs a VertexBuffer from the CPUVertexArray, and renders it using a simple shader that
    mimics the old fixed-function pipeline */
void SlowMesh::render(RenderDevice* rd, int coverageSamples) const {
    debugAssertGLOk();

    const static bool hasRasterSamples = GLCaps::supports("GL_EXT_raster_multisample");
    if (m_cpuVertexArray.size() > 0) {
        if (coverageSamples > 1 && ! hasRasterSamples) {
            // For older Intel chipsets
            coverageSamples = 1;
        }

        AttributeArray vertex, normal, texCoord0, vertexColors;
        copyToGPU(vertex, normal, texCoord0, vertexColors);
        Args args;
        if (notNull(m_texture)) {
            args.setMacro("HAS_TEXTURE", 1);
            args.setUniform("textureMap", m_texture, Sampler::video());
        } else {
            args.setMacro("HAS_TEXTURE", 0);
        }
            
        args.setMacro("IS_PROJECTION", rd->projectionMatrix()[3][4] == 0.0f ? 1 : 0);
        args.setUniform("pointSize", m_pointSize);

        args.setPrimitiveType(m_primitiveType);
        args.setAttributeArray("g3d_Vertex", vertex);
        args.setAttributeArray("g3d_Normal", normal);
        args.setAttributeArray("g3d_TexCoord0", texCoord0);
        args.setAttributeArray("g3d_Color", vertexColors);

        if (coverageSamples > 1) {
            args.setMacro("COVERAGE_SAMPLES", coverageSamples);
            glRasterSamplesEXT(coverageSamples, GL_TRUE);
            glEnable(GL_RASTER_MULTISAMPLE_EXT);
        }

        LAUNCH_SHADER("SlowMesh_render.*", args);
        if (coverageSamples > 1) {
            glDisable(GL_RASTER_MULTISAMPLE_EXT);
        }
    }
}

void SlowMesh::reserveSpace(int numVertices) {
    m_cpuVertexArray.reserve(numVertices);
}
}
