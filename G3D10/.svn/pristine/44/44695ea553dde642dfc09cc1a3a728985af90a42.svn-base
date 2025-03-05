/**
  \file G3D-gfx.lib/include/G3D-gfx/G3D-gfx.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define G3D_gfx_G3D_gfx_h

#include "G3D-base/G3D-base.h"
#include "G3D-gfx/glheaders.h"
#include "G3D-gfx/glcalls.h"
#include "G3D-gfx/getOpenGLState.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/glFormat.h"
#include "G3D-gfx/Milestone.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/VertexBuffer.h"
#include "G3D-gfx/AttributeArray.h"
#include "G3D-gfx/tesselate.h"
#include "G3D-gfx/OSWindow.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-gfx/GKey.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-gfx/CPUVertexArray.h"
#include "G3D-gfx/GLFWWindow.h"
#include "G3D-gfx/Args.h"
#include "G3D-gfx/Shader.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-gfx/BufferTexture.h"
#include "G3D-gfx/BindlessTextureHandle.h"
#include "G3D-gfx/XR.h"
#include "G3D-gfx/GazeTracker.h"
#include "G3D-gfx/OpenVR.h"
#include "G3D-gfx/MonitorXR.h"
#include "G3D-gfx/VideoStream.h"

#ifdef G3D_WINDOWS
#include "G3D-gfx/DXCaps.h"
#endif

#ifndef G3D_NO_FMOD
#include "G3D-gfx/AudioDevice.h"
#endif

#ifdef GLEW_EGL
#include "G3D-gfx/EGLWindow.h"
#endif


namespace G3D {
    /** 
      Call from main() to initialize the GLG3D library state and register
      shutdown memory managers.  This does not initialize OpenGL. 

      This automatically calls initG3D. It is safe to call this function 
      more than once--it simply ignores multiple calls.

      \see GLCaps, OSWindow, RenderDevice, initG3D
    */
    void initGLG3D(const G3DSpecification& spec = G3DSpecification());
}

// Set up the linker on Windows
#ifdef _MSC_VER

#   pragma comment(lib, "ole32")
#   pragma comment(lib, "opengl32")
#   pragma comment(lib, "glu32")
#   pragma comment(lib, "shell32") // for drag & drop

#   pragma comment(lib, "glew")
#   pragma comment(lib, "glfw")
 // Vulkan
#   pragma comment(lib, "vulkan-1")

#   ifndef G3D_NO_FMOD
#      ifdef G3D_WINDOWS
#          pragma comment(lib, "fmod_vc.lib")
#      endif
#   endif

#   ifdef _DEBUG
// glslang
#      pragma comment(lib, "glslangd")
#      pragma comment(lib, "SPIRVd")
#      pragma comment(lib, "SPVRemapperd")
#      pragma comment(lib, "OSDependentd")
#      pragma comment(lib, "OGLCompilerd")
#      pragma comment(lib, "G3D-gfxd")
#   else
#      pragma comment(lib, "glslang")
#      pragma comment(lib, "SPIRV")
#      pragma comment(lib, "SPVRemapper")
#      pragma comment(lib, "OSDependent")
#      pragma comment(lib, "OGLCompiler")
#      pragma comment(lib, "G3D-gfx")
#   endif

#endif
