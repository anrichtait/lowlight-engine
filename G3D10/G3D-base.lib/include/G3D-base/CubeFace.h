/**
  \file G3D-base.lib/include/G3D-base/CubeFace.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef CubeFace_h
#define CubeFace_h

#include "G3D-base/platform.h"
#include "G3D-base/enumclass.h"

namespace G3D {

class CubeFace {
public:
    /** Don't use this enum; use CubeFace instances instead. */
    enum Value {
        POS_X = 0,
        NEG_X = 1,
        POS_Y = 2,
        NEG_Y = 3,
        POS_Z = 4,
        NEG_Z = 5
    };
private:
    
    static const char* toString(int i, Value& v) {
        static const char* str[] = {"POS_X", "NEG_X", "POS_Y", "NEG_Y", "POS_Z", "NEG_Z", nullptr}; 
        const char* s = str[i];
        if (s) {
            v = Value(i);
        }
        return s;
    }

    Value value;

public:

    G3D_DECLARE_ENUM_CLASS_METHODS(CubeFace);

};



/** Image alignment conventions specified by different APIs. 
    G3D loads cube maps so that they act like reflection maps.
    That is, it assumes you are <i>inside</i> the cube map.
    */
class CubeMapConvention {
public:

    struct CubeMapInfo {
        struct Face {
            /** True if the face is horizontally flipped */
            bool            flipX;

            /** True if the face is vertically flipped */
            bool            flipY;

            /** Number of CW 90-degree rotations to perform after flipping */
            int             rotations;

            /** Filename suffix */
            String          suffix;

            Face() : flipX(true), flipY(false), rotations(0) {}
        };
        
        String              name;

        /** Index using CubeFace */
        Face                face[6];
    };

    enum Value {
        /** Uses "up", "lf", etc. */
        QUAKE, 
        
        /** Uses "up", "west", etc. */
        UNREAL, 

        /** Uses "+y", "-x", etc. */
        G3D,

        /** Uses "PY", "NX", etc. */
        DIRECTX
    };
        
private:
    
    static const char* toString(int i, Value& v) {
        static const char* str[] = {"QUAKE", "UNREAL", "G3D", "DIRECTX", nullptr}; 
        const char* s = str[i];
        if (s) {
            v = Value(i);
        }
        return s;
    }

    Value value;

public:

    G3D_DECLARE_ENUM_CLASS_METHODS(CubeMapConvention);

    /** Number of conventions supported */
    enum {COUNT = int(DIRECTX) + 1};
};

} // namespace G3D

G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::CubeFace);
G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::CubeMapConvention);

#endif
