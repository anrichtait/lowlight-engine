/**
  \file G3D-app.lib/source/ArticulatedModel_Schematic.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/ArticulatedModel.h"
//#include "G3D-base/ParseSchematic.h"
#include "G3D-base/Vector3uint8.h"
#include "G3D-base/Vector3int16.h"
#include "G3D-base/FastPODTable.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/MeshAlg.h" //Not sure if we need this yet

namespace G3D {

// C++ Increment Mod 3 [nextMod3] from http://graphicscodex.com 
static inline int incMod3(const int i) {
   // The bitwise operations implement ($i$ + 1) % 3 without the modulo
   return (1 << i) & 3;
}


void ArticulatedModel::loadSchematic(const Specification& specification){

    ParseSchematic parseData;
    parseData.parse(specification.filename);

    ParseSchematic::ColorVoxels voxelGrid;
    parseData.getSparseVoxelTable(voxelGrid);

    addVoxels(voxelGrid, Point3int32(0,0,0), parseData.size - Vector3int32(1,1,1), specification);
}

void ArticulatedModel::addVoxels(const ParseSchematic::ColorVoxels& voxels, Point3int32 minBound, Point3int32 maxBound, const Specification& specification) {
    Part*       part = addPart(m_name);
    Geometry*   geom = addGeometry("geom");
    Mesh*       mesh = addMesh("mesh", part, geom);
    mesh->material = UniversalMaterial::create();


    const float EDGE_LENGTH = 0.01f;

    geom->cpuVertexArray.hasTangent = false;
    geom->cpuVertexArray.hasTexCoord0 = false;
    geom->cpuVertexArray.hasVertexColors = true;
    Array<int>& indexArray = mesh->cpuIndexArray;
    Array<CPUVertexArray::Vertex>& vertexArray = geom->cpuVertexArray.vertex;
    // const unorm8 solidAlpha = unorm8::fromBits(255);


    // For each voxel
    for (ParseSchematic::ColorVoxels::ConstIterator it = voxels.begin(); it.isValid(); ++it) {
        const Point3int16 position(it.key());
        Color4 color = it.value();
        // Gamma correct
        color = Color4(color.rgb() * color.rgb(), 1.0);//color.a);

                                                       // For each cube face of the voxel
        for (int face = 0; face < 6; ++face) {
            // Create the local coordinate system
            const int axis = face >> 1;
            Vector3 normal;
            const int int_d = (face & 1) * 2 - 1;
            const float d = float(int_d);
            normal[axis] = d;

            Point3int16 neighborPosition = position;
            neighborPosition[axis] += int_d;


            // Check the neighbor, unless it is out of bounds
            bool neighborIsOpaque = !specification.voxelOptions.removeInternalVoxels ||
                (specification.voxelOptions.treatBorderAsOpaque &&
                ((neighborPosition.x == maxBound.x + 1) || (neighborPosition.x < 0) || //Add 1 to reach parseData.size
                    (neighborPosition.z == maxBound.z + 1) || (neighborPosition.z < 0) ||
                    (neighborPosition.y < 0)));

            neighborIsOpaque = neighborIsOpaque || voxels.containsKey(neighborPosition);

            if (!neighborIsOpaque /* || (color.a != 1.0f) ... we don't support alpha yet */) {
                const int n = vertexArray.size();
                vertexArray.resize(n + 4);

                CPUVertexArray::Vertex& v0 = vertexArray[n];
                CPUVertexArray::Vertex& v1 = vertexArray[n + 1];
                CPUVertexArray::Vertex& v2 = vertexArray[n + 2];
                CPUVertexArray::Vertex& v3 = vertexArray[n + 3];

                const Point3& center = (Point3(position) + normal * 0.5f) * EDGE_LENGTH;
                Vector3 X, Y;
                X[incMod3(axis)] = EDGE_LENGTH * 0.5f * d;
                Y[incMod3(incMod3(axis))] = EDGE_LENGTH * 0.5f;

                v0.position = center + X + Y;
                v1.position = center - X + Y;
                v2.position = center - X - Y;
                v3.position = center + X - Y;

                v0.normal = v1.normal = v2.normal = v3.normal = normal;
                geom->cpuVertexArray.vertexColors.append(color, color, color, color);

                indexArray.append(n, n + 1, n + 2,
                    n, n + 2, n + 3);
            } // If this face is externally-visible
        }
    }
}

} // namespace G3D
