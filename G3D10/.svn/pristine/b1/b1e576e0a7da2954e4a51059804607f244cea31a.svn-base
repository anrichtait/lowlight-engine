/**
  \file G3D-app.lib/source/ArticulatedModel_heightfield.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/ArticulatedModel.h"
#include "G3D-base/FileSystem.h"

namespace G3D {

void ArticulatedModel::loadHeightfield(const Specification& specification) {
    Part* part = addPart("root");
    Geometry* geom = addGeometry("geom");
    Mesh* mesh = addMesh("mesh", part, geom);
    mesh->material = UniversalMaterial::create();
    
    const shared_ptr<Image1>& im = Image1::fromFile(specification.filename);
                
    geom->cpuVertexArray.hasTangent = false;
    geom->cpuVertexArray.hasTexCoord0 = true;
    
    const bool spaceCentered = true;
    const float elevationScale = specification.heightfieldOptions.elevationScale;
    const bool generateBackFaces = specification.heightfieldOptions.generateBackfaces;
    const Vector2& textureScale  = specification.heightfieldOptions.textureScale;

    Array<Point3> vertex;
    Array<Point2> texCoord;
    MeshAlg::generateGrid
        (vertex, texCoord, mesh->cpuIndexArray, 
        im->width(), im->height(), textureScale,
        spaceCentered,
        generateBackFaces,
        CFrame(Matrix4::scale((float)im->width(), elevationScale, (float)im->height()).upper3x3()),
        im);


    // Copy the vertex data into the mesh
    geom->cpuVertexArray.vertex.resize(vertex.size());
    CPUVertexArray::Vertex* vertexPtr = geom->cpuVertexArray.vertex.getCArray();
    for (uint32 i = 0; i < (uint32)vertex.size(); ++i) {
        CPUVertexArray::Vertex& v = vertexPtr[i];
        v.position = vertex[i];
        v.texCoord0 = texCoord[i];
        v.tangent.x = v.normal.x = fnan();
    } // for
}

} // namespace G3D
