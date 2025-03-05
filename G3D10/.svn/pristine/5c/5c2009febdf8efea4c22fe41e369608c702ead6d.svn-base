/**
  \file G3D-base.lib/include/G3D-base/FrameName.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_FrameName_h
#define G3D_FrameName_h

#include "G3D-base/platform.h"
#include "G3D-base/enumclass.h"

namespace G3D {

/** \def FrameName

Logical reference frame.

NONE: Does not apply or unknown

WORLD: Scene's reference frame
        
OBJECT: A.k.a. body. For VR, this is the player's body

CAMERA: Camera's object space. For VR, this is the eye space

LIGHT: Light's object space 

TANGENT: Surface space. Texture space embedded in object space, i.e., relative to a unique 3D space that
    conforms to the object's surface with the Z axis pointing along the surface normal,
    and with a unique translation for every object space location.

TEXTURE: The object's 2D "uv" parameterization surface space. 

SCREEN: Pixels, with upper-left = (0, 0)
*/
G3D_DECLARE_ENUM_CLASS(FrameName, NONE, WORLD, OBJECT, CAMERA, LIGHT, TANGENT, TEXTURE, SCREEN);
}

G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::FrameName);

#endif
