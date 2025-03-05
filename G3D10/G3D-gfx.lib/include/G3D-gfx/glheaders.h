/**
  \file G3D-gfx.lib/include/G3D-gfx/glheaders.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_glHeaders_h

#include "G3D-base/platform.h"
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include "GL/glew.h"

#ifdef G3D_WINDOWS
#   include "GL/wglew.h"
#elif defined(G3D_FREEBSD) || defined(G3D_LINUX)
#   include "GL/glxew.h"
#endif

#ifdef G3D_OSX
#    include <OpenGL/glu.h>
#    include <OpenGL/OpenGL.h>
#endif

// needed for OS X header
#ifdef Status
#undef Status

#ifndef GL_VERSION_4_5
#   error GLG3D Requires OpenGL 4.5 or later header files distributed with G3D. You probably have an old version of GLEW or GLUT in your include path.
#endif

#endif

