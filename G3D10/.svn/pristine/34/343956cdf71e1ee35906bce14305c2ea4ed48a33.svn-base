/**
  \file G3D-app.lib/source/ArticulatedModel_VOX.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/ArticulatedModel.h"
#include "G3D-base/ParseVOX.h"
#include "G3D-base/Vector3uint8.h"
#include "G3D-base/FastPODTable.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/MeshAlg.h" //Not sure if we need this yet

namespace G3D {


void ArticulatedModel::loadVOX(const Specification& specification) {

    ParseVOX parseData;
    {
        BinaryInput bi(specification.filename, G3D_LITTLE_ENDIAN);
        parseData.parse(bi);
    }

    ParseSchematic::ColorVoxels voxels;

    // Keep track of these to determine the size of the model
    // for duplicating and for handling boundary case
    Point3int32 minBound(32767, 32767, 32767), maxBound(-32767, -32767, -32767);

    // Convert from Point3uint8 to Point3int16 indices and swap coordinate system
    for (const ParseVOX::Voxel v : parseData.voxel) {
        const Point3int16 position(v.position.x, v.position.z, -v.position.y);
        voxels[position] = parseData.palette[v.index];
        minBound = minBound.min(position);
        maxBound = maxBound.max(position);
    }

    addVoxels(voxels, minBound, maxBound, specification);
}


} // namespace G3D
