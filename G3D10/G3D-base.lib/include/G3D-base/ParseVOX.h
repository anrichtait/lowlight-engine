/**
  \file G3D-base.lib/include/G3D-base/ParseVOX.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once


#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/Color4unorm8.h"
#include "G3D-base/Vector3uint8.h"
#include "G3D-base/Vector3int32.h"

namespace G3D {

class BinaryInput;

/** Parser for the MagicaVoxel sparse voxel format
    http://ephtracy.github.io/index.html?page=mv_vox_format */
class ParseVOX {
public:

    class Voxel {
    public:
        Point3uint8             position;

        /** Index into palette */
        uint8                   index;
    };
#if 0
    typedef FastPODTable<Point3uint8, uint8, HashTrait<Point3uint8>, EqualsTrait<Point3uint8>, true> VoxelTable;

    /** voxel[xyz] is an index into the palette. Non-zero voxels can be obtained by voxel.begin().
        Use voxel.getPointer(pos) to test whether a voxel is present without creating a new empty
        value at that location.*/
    VoxelTable                  voxel;
#endif

    Array<Voxel>                voxel;

    /** These are shifted from the file format, so that palette[0] 
        is always transparent black and palette[1] is the first value
        from the palette file.*/
    Color4unorm8                palette[256];

    Vector3int32                size;

    void parse(const char* ptr, size_t len);

    void parse(BinaryInput& bi);
};

}

