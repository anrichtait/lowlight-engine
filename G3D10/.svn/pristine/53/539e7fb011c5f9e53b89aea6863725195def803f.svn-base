/**
  \file G3D-gfx.lib/include/G3D-gfx/getOpenGLState.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_getOpenGLState_h

#include "G3D-base/platform.h"
#include "./glheaders.h"
#include "./glcalls.h"

namespace G3D {

/** 
 Returns all OpenGL state as a formatted string of C++
 code that will reproduce that state.   Leaves all OpenGL
 state in exactly the same way it found it.   Use this for 
 debugging when OpenGL doesn't seem to be in the same state 
 that you think it is in. 

  A common idiom is:
  {String s = getOpenGLState(false); debugPrintf("%s", s.c_str();}

 @param showDisabled if false, state that is not affecting
  rendering is not shown (e.g. if lighting is off,
  lighting information is not shown).
 */
String getOpenGLState(bool showDisabled=true);


/**
 Returns the number of bytes occupied by a value in
 an OpenGL format (e.g. GL_FLOAT).  Returns 0 for unknown
 formats.
 */
int sizeOfGLFormat(GLenum format);

/**
 Pretty printer for GLenums.  Useful for debugging OpenGL
 code.
 */
const char* GLenumToString(GLenum i);

}
