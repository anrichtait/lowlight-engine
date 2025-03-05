#pragma once
#include <G3D/G3D.h>

/** A mesh that renders with a solid base and a "wireframe" outline */
class WireMesh : public ReferenceCountedObject {
protected:

    static const size_t             s_maxVertexCount = 1024 * 20;
    static const size_t             s_maxIndexCount = 1024 * 20;

    static shared_ptr<VertexBuffer> s_gpuBuffer;
    static AttributeArray           s_position;
    static AttributeArray           s_color;
    static size_t                   s_vertexCount;

    /** in sRGB */
    Color3                          m_solidColor;

    /** in sRGB */
    Color3                          m_edgeColor;

    /** If s_vertexBuffer was smaller, then we could use 16-bit indices
        which fetch at 2x speed on NVIDIA GPUs. However, the geometry in
        this program is so simple that it makes more sense to minimize
        CPU-GPU attribute binding changes using monolithic buffers than to optimize
        for GPU attribute fetch. */
    IndexStream                     m_indexStream;

    WireMesh(const Color3& s, const Color3& e) : m_solidColor(s), m_edgeColor(e) {}

public:

    static shared_ptr<WireMesh> create(const String& geometryFilename, float scale, const Color3& solidColor, const Color3& wireColor);
    static void render(RenderDevice* rd, const Array<shared_ptr<WireMesh>>& array, const Array<CFrame>& cframe);
};
