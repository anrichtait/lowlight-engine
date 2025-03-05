#include "WireMesh.h"

shared_ptr<VertexBuffer> WireMesh::s_gpuBuffer;
AttributeArray           WireMesh::s_position;
AttributeArray           WireMesh::s_color;
size_t                   WireMesh::s_vertexCount = 0;


/** Extracts just the mesh information */
static void loadGeometry(const String& filename, float scale, Color3 faceColor, Array<Point3>& position, Array<bool>& flag, Array<int>& index) {
    ArticulatedModel::Specification spec;

    // Merge all geometry
    spec.filename = filename;
    spec.stripMaterials = true;
    spec.stripVertexColors = true;
    spec.scale = scale;
    spec.cleanGeometrySettings.allowVertexMerging = true;
    spec.cleanGeometrySettings.forceVertexMerging = true;
    spec.cleanGeometrySettings.maxNormalWeldAngle = finf();
    spec.cleanGeometrySettings.maxSmoothAngle = finf();
    spec.meshMergeOpaqueClusterRadius = finf();

    const shared_ptr<ArticulatedModel>& model = ArticulatedModel::create(spec);
    Array<shared_ptr<Surface>> surfaceArray;        
    model->pose(surfaceArray, CFrame(), CFrame(), nullptr, nullptr, nullptr, Surface::ExpressiveLightScatteringProperties());
        
    Array<Tri> triArray;
    CPUVertexArray vertexArray;
    Surface::getTris(surfaceArray, vertexArray, triArray);

    position.resize(vertexArray.size());
    flag.resize(vertexArray.size());
    for (int v = 0; v < vertexArray.size(); ++v) {
        position[v] = vertexArray.vertex[v].position;
        flag[v] = false;
    }

    index.resize(triArray.size() * 3);
    for (int t = 0; t < triArray.size(); ++t) {
        const Tri& tri = triArray[t];
        for (int v = 0; v < 3; ++v) {
            index[t * 3 + v] = tri.getIndex(v);
        }
    }
}


static void appendFeatureEdges(float thickness, Array<Point3>& positionArray, Array<bool>& flagArray, Array<int>& indexArray) {
    Array<MeshAlg::Face> faceArray;
    Array<MeshAlg::Edge> edgeArray;
    Array<MeshAlg::Vertex> vertexArray;
    MeshAlg::computeAdjacency(positionArray, indexArray, faceArray, edgeArray, vertexArray);

    // Merge colocated vertices (this was mostly done on load, but
    // there may still be some separate ones due to texture coordinates)
    MeshAlg::weldAdjacency(positionArray, faceArray, edgeArray, vertexArray);

    // Compute a flag on all feature edges
    Array<bool> featureEdge;
    Array<Vector3> faceNormalArray;
    MeshAlg::computeFaceNormals(positionArray, faceArray, faceNormalArray);
    MeshAlg::identifyFeatureEdges(positionArray, edgeArray, faceArray, faceNormalArray, featureEdge, 2.1f);

    // For each feature edge, generate a quad that is its edge. Since we're sharing
    // no vertices with
    MeshBuilder builder;
    for (const MeshAlg::Face& face : faceArray) {
        // For each feature edge:
        //      Compute the start vertex:
        //          If the next edge in the poly is a feature edge, place the vertex between them
        //          Else put it on the next edge
        //      Compute the end vertex
        // TODO
    }

    // Append the new geometry to the input arrays
    // TODO
}

shared_ptr<WireMesh> WireMesh::create(const String& geometryFilename, float scale, const Color3& solidColor, const Color3& wireColor) {
    const shared_ptr<WireMesh>& mesh = createShared<WireMesh>(solidColor, wireColor);

    const size_t stride = sizeof(Point3) + sizeof(Color3);
    if (isNull(s_gpuBuffer)) {
        const size_t maxVertexBytes = s_maxVertexCount * sizeof(Vector4);
        const size_t maxGPUBufferSizeBytes = maxVertexBytes + s_maxIndexCount * sizeof(int);

        // Allocate the vertex buffer with space for *all* meshes
        s_gpuBuffer = VertexBuffer::create(maxGPUBufferSizeBytes, VertexBuffer::WRITE_ONCE);
        s_vertexCount = 0;
        
        s_position = AttributeArray(Vector4(), s_maxVertexCount, s_gpuBuffer);

        // (The remaining space in s_gpuBuffer is for the index arrays, which are allocated in each create() call)
    }

    // Load the geometry
    Array<Point3> positionArray;
    // true for the solid surfaces and false for the wireframe surfaces
    Array<bool> flagArray;
    Array<int> indexArray;
    loadGeometry(geometryFilename, scale, solidColor, positionArray, flagArray, indexArray);

    // Compute additional feature edges for "wireframe" rendering
    appendFeatureEdges(0.015f * scale, positionArray, flagArray, indexArray);   

    // Upload to the shared, interleaved arrays
    {
        Vector4* buffer = (Vector4*)s_position.mapBuffer(GL_WRITE_ONLY);
        for (int v = 0; v < positionArray.size(); ++v) {
            buffer[v] = Vector4(positionArray[v], flagArray[v] ? 1.0f : 0.0f);
        }
        s_position.unmapBuffer();
    }

    // Construct the index stream
    mesh->m_indexStream = IndexStream(indexArray, s_gpuBuffer);
    return mesh;
}


void WireMesh::render(RenderDevice* rd, const Array<shared_ptr<WireMesh>>& array, const Array<CFrame>& cframe) {
    debugAssertM(array.size() == cframe.size(), "Must have the same number of coordinate frames and meshes");
    rd->pushState(); {

        // Processing all of the meshes at once allows minimizing state changes. We could perform instanced
        // rendering for an even greater speedup if there were many copies of a single mesh. We could also
        // use AMD-style "pulling" to submit the entire scene as one draw call with a vertex shader that 
        // reads from a texture, but the draw call overhead is sufficiently low that we can still hit
        // 1000 Hz with this method and those extremes aren't necessary.

        Args args;

        // Global attribute arrays
        args.setAttributeArray("g3d_Vertex", s_position);

        // Per-mesh draw calls
        for (int i = 0; i < array.size(); ++i) {
            const shared_ptr<WireMesh>& mesh = array[i];
            rd->setObjectToWorldMatrix(cframe[i]);
            args.setUniform("edgeColor", mesh->m_edgeColor);
            args.setUniform("solidColor", mesh->m_solidColor);
            args.setIndexStream(array[i]->m_indexStream);
            LAUNCH_SHADER("WireMesh.*", args);
        }
    } rd->popState();
}
