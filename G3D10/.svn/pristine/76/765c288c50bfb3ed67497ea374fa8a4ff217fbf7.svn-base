/**
  \file G3D-base.lib/include/G3D-base/G3D-base.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_base_G3D_base_h
#ifndef NOMINMAX
    #define NOMINMAX 1
#endif
#ifdef min
    #undef min
#endif
#ifdef max
    #undef max
#endif

#include "G3D-base/DoNotInitialize.h"
#include "G3D-base/platform.h"
#include "G3D-base/HaltonSequence.h"
#include "G3D-base/lazy_ptr.h"
#include "G3D-base/BIN.h"
#include "G3D-base/FileNotFound.h"
#include "G3D-base/units.h"
#include "G3D-base/ParseError.h"
#include "G3D-base/Random.h"
#include "G3D-base/Noise.h"
#include "G3D-base/Array.h"
#include "G3D-base/SmallArray.h"
#include "G3D-base/Queue.h"
#include "G3D-base/TimeQueue.h"
#include "G3D-base/Crypto.h"
#include "G3D-base/format.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/Vector2int32.h"
#include "G3D-base/Vector2uint32.h"
#include "G3D-base/Vector2int16.h"
#include "G3D-base/Vector2uint16.h"
#include "G3D-base/Vector2unorm16.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/Vector3int16.h"
#include "G3D-base/Vector3int32.h"
#include "G3D-base/Vector4.h"
#include "G3D-base/Vector4int16.h"
#include "G3D-base/Vector4uint16.h"
#include "G3D-base/Vector4int8.h"
#include "G3D-base/Color1.h"
#include "G3D-base/Color3.h"
#include "G3D-base/Color4.h"
#include "G3D-base/Matrix2.h"
#include "G3D-base/Matrix3.h"
#include "G3D-base/Matrix2x3.h"
#include "G3D-base/Matrix3x4.h"
#include "G3D-base/Matrix4.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/Projection.h"
#include "G3D-base/PhysicsFrame.h"
#include "G3D-base/PhysicsFrameSpline.h"
#include "G3D-base/Plane.h"
#include "G3D-base/Line.h"
#include "G3D-base/Ray.h"
#include "G3D-base/Sphere.h"
#include "G3D-base/Box.h"
#include "G3D-base/Box2D.h"
#include "G3D-base/AABox.h"
#include "G3D-base/WrapMode.h"
#include "G3D-base/CullFace.h"
#include "G3D-base/Cone.h"
#include "G3D-base/Quat.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/prompt.h"
#include "G3D-base/Table.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/Set.h"
#include "G3D-base/GUniqueID.h"
#include "G3D-base/RayGridIterator.h"
#include "G3D-base/BinaryFormat.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/debug.h"
#include "G3D-base/g3dfnmatch.h"
#include "G3D-base/G3DGameUnits.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/unorm8.h"
#include "G3D-base/unorm16.h"
#include "G3D-base/snorm8.h"
#include "G3D-base/snorm16.h"
#include "G3D-base/uint128.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/Welder.h"
#include "G3D-base/PrecomputedRandom.h"
#include "G3D-base/MemoryManager.h"
#include "G3D-base/BlockPoolMemoryManager.h"
#include "G3D-base/AreaMemoryManager.h"
#include "G3D-base/BumpMapPreprocess.h"
#include "G3D-base/CubeFace.h"
#include "G3D-base/Line2D.h"
#include "G3D-base/ThreadsafeQueue.h"
#include "G3D-base/network.h"
#include "G3D-base/FrameName.h"
#include "G3D-base/G3DAllocator.h"
#include "G3D-base/OrderedTable.h"
#include "G3D-base/Journal.h"
#include "G3D-base/Grid.h"
#include "G3D-base/Pathfinder.h"
#include "G3D-base/EqualsTrait.h"
#include "G3D-base/Image.h"
#include "G3D-base/CubeMap.h"
#include "G3D-base/CollisionDetection.h"
#include "G3D-base/Intersect.h"
#include "G3D-base/Log.h"
#include "G3D-base/serialize.h"
#include "G3D-base/TextInput.h"
#include "G3D-base/NetAddress.h"
#include "G3D-base/NetworkDevice.h"
#include "G3D-base/System.h"
#include "G3D-base/splinefunc.h"
#include "G3D-base/Spline.h"
#include "G3D-base/UprightFrame.h"
#include "G3D-base/LineSegment.h"
#include "G3D-base/Capsule.h"
#include "G3D-base/Cylinder.h"
#include "G3D-base/Triangle.h"
#include "G3D-base/Color1unorm8.h"
#include "G3D-base/Color2unorm8.h"
#include "G3D-base/Color3unorm8.h"
#include "G3D-base/Color4unorm8.h"
#include "G3D-base/ConvexPolyhedron.h"
#include "G3D-base/MeshAlg.h"
#include "G3D-base/vectorMath.h"
#include "G3D-base/Rect2D.h"
#include "G3D-base/KDTree.h"
#include "G3D-base/PointKDTree.h"
#include "G3D-base/TextOutput.h"
#include "G3D-base/MeshBuilder.h"
#include "G3D-base/Stopwatch.h"
#include "G3D-base/Thread.h"
#include "G3D-base/RegistryUtil.h"
#include "G3D-base/Any.h"
#include "G3D-base/XML.h"
#include "G3D-base/PointHashGrid.h"
#include "G3D-base/Map2D.h"
#include "G3D-base/Image1.h"
#include "G3D-base/Image1unorm8.h"
#include "G3D-base/Image3.h"
#include "G3D-base/Image3unorm8.h"
#include "G3D-base/Image4.h"
#include "G3D-base/Image4unorm8.h"
#include "G3D-base/filter.h"
#include "G3D-base/WeakCache.h"
#include "G3D-base/Pointer.h"
#include "G3D-base/Matrix.h"
#include "G3D-base/ImageFormat.h"
#include "G3D-base/PixelTransferBuffer.h"
#include "G3D-base/typeutils.h"
#include "G3D-base/ParseMTL.h"
#include "G3D-base/ParseOBJ.h"
#include "G3D-base/ParsePLY.h"
#include "G3D-base/Parse3DS.h"
#include "G3D-base/PathDirection.h"
#include "G3D-base/FastPODTable.h"
#include "G3D-base/ParseVOX.h"
#include "G3D-base/ParseSchematic.h"
#include "G3D-base/FastPointHashGrid.h"
#include "G3D-base/PixelTransferBuffer.h"
#include "G3D-base/CPUPixelTransferBuffer.h"
#include "G3D-base/CompassDirection.h"
#include "G3D-base/Access.h"
#include "G3D-base/DepthFirstTreeBuilder.h"
#include "G3D-base/SmallTable.h"
#include "G3D-base/float16.h"
#include "G3D-base/PrefixTree.h"
#include "G3D-base/WebServer.h"

namespace G3D {

    /** 
      Call from main() to initialize the G3D library state and register
      shutdown memory managers.  This does not initialize OpenGL. 

      If you invoke initGLG3D, then it will automatically call initG3D.
      It is safe to call this function more than once--it simply ignores
      multiple calls.

      \see System, GLCaps, OSWindow, RenderDevice, initGLG3D.
        
    */
    void initG3D(const G3DSpecification& spec = G3DSpecification());
}

#ifdef _MSC_VER
#   pragma comment(lib, "winmm")
#   pragma comment(lib, "imagehlp")
#   pragma comment(lib, "ws2_32")
#   pragma comment(lib, "gdi32")
#   pragma comment(lib, "user32")
#   pragma comment(lib, "kernel32")
#   pragma comment(lib, "advapi32")
#   pragma comment(lib, "shell32")
#   pragma comment(lib, "version")
#   pragma comment(lib, "tbb.lib")
#   pragma comment(lib, "zlib")
#   pragma comment(lib, "zip")
#   pragma comment(lib, "enet")
#   if defined(_DEBUG)
#       pragma comment(lib, "G3D-based")
#       pragma comment(lib, "freeimaged")
#       pragma comment(lib, "civetwebd")
#   else
#       pragma comment(lib, "G3D-base")
#       pragma comment(lib, "freeimage")
#       pragma comment(lib, "civetweb")
#   endif
#endif
