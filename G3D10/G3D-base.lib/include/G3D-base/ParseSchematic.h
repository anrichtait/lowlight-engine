/**
  \file G3D-base.lib/include/G3D-base/ParseSchematic.h

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
#include "G3D-base/Vector3int16.h"
#include "G3D-base/FastPODTable.h"

namespace G3D {

class BinaryInput;

/** Parser for the MagicaVoxel sparse voxel format
    http://ephtracy.github.io/index.html?page=mv_vox_format */
class ParseSchematic {
public:
    /** Palette struct for entries of Mineways single color voxel conversion table **/
    struct SchematicPalette {
        const char*             name;
        uint32                  color;
        float                   alpha;
        int                     texIndexX;
        int                     texIndexY;
        bool                    emissive;
    };
    
    Point3int32                 size;
    String                      materials;

    /** Raw data grid in YZX-major order. */
    Array<uint8>                blockID;

    /** 4-bit lighting data */
    Array<uint8>                blockData;

    static Color4unorm8 schematicBlockColor(uint8 blockIndex);
    void parseSchematicTag(BinaryInput& bi, int processTagType = -1);
    void parse(const String& filename);

    typedef FastPODTable<Point3int16, uint8, HashTrait<Point3int16>, EqualsTrait<Point3int16>, true>
        BlockIDVoxels;

    typedef FastPODTable<Point3int16, Color4unorm8, HashTrait<Point3int16>, EqualsTrait<Point3int16>, true>
        ColorVoxels;

    void getSparseVoxelTable(BlockIDVoxels& voxel) const;
    void getSparseVoxelTable(ColorVoxels& voxel) const;
};
} // G3D