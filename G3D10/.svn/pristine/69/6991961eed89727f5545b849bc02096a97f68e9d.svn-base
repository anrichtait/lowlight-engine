/**
  \file G3D-app.lib/include/G3D-app/debugDraw.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_debugDraw_h

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/Color4.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-app/GFont.h"

namespace G3D {

class Shape;
class Box;
class Tri;
class GuiText;
class CPUVertexArray;

/** Used with debugDraw. */
typedef int DebugID;

/**
 \brief Schedule a G3D::Shape for later rendering.

 Adds this shape and the specified information to the current G3D::GApp::debugShapeArray,
 to be rendered at runtime for debugging purposes.

 Sample usage is:
 \code
 debugDraw(new SphereShape(Sphere(center, radius)));
 \endcode

 

 \param displayTime Real-world time in seconds to display the shape
 for.  A shape always displays for at least one frame. 0 = one frame.
 inf() = until explicitly removed by the GApp.

 \return The ID of the shape, which can be used to clear it for shapes that are displayed "infinitely".

 \sa debugPrintf, logPrintf, screenPrintf, GApp::drawDebugShapes, GApp::removeDebugShape, GApp::removeAllDebugShapes
 */
DebugID debugDraw
(const shared_ptr<Shape>&           shape,
 float                              displayTime = 0.0f,
 const Color4&                      solidColor  = Color3::white(),
 const Color4&                      wireColor   = Color3::black(),
 const CoordinateFrame&             cframe      = CoordinateFrame());

/** overloaded forms of debugDraw so that more common parameters can be passed (e.g Boxes, Spheres) */
DebugID debugDraw
(const Box&                         b,
 float                              displayTime = 0.0f,
 const Color4&                      solidColor  = Color3::white(),
 const Color4&                      wireColor   = Color3::black(),
 const CoordinateFrame&             cframe      = CoordinateFrame());

DebugID debugDraw
(const Array<Vector3>&              vertices,
 const Array<int>&                  indices,
 float                              displayTime = 0.0f,
 const Color4&                      solidColor  = Color3::white(),
 const Color4&                      wireColor   = Color3::black(),
 const CoordinateFrame&             cframe      = CoordinateFrame());

DebugID debugDraw
(const CPUVertexArray&              vertices,
 const Array<Tri>&                  tris,
 float                              displayTime = 0.0f,
 const Color4&                      solidColor  = Color3::white(),
 const Color4&                      wireColor   = Color3::black(),
 const CoordinateFrame&             cframe      = CoordinateFrame());

DebugID debugDraw
(const Sphere&                      s,
 float                              displayTime = 0.0f,
 const Color4&                      solidColor  = Color3::white(),
 const Color4&                      wireColor   = Color3::black(),
 const CoordinateFrame&             cframe      = CoordinateFrame());

/** Drawing of Points implemented as drawing small spheres */
DebugID debugDraw
(const Point3&                      p,
 float                              displayTime = 0.0f,
 const Color4&                      solidColor  = Color3::white(),
 const Color4&                      wireColor   = Color3::black(),
 const CoordinateFrame&             cframe      = CoordinateFrame());

DebugID debugDraw
(const CoordinateFrame&             cf,
 float                              displayTime = 0.0f,
 const Color4&                      solidColor  = Color3::white(),
 const Color4&                      wireColor   = Color3::black(),
 const CoordinateFrame&             cframe      = CoordinateFrame());

/** Draws a label onto the screen for debug purposes*/
DebugID debugDrawLabel
(const Point3&                      wsPos,
 const Vector3&                     csOffset,
 const GuiText&                     text,
 float                              displayTime = 0.0f,
 float                              size = 0.1f, //size in meters, by default
 bool                               sizeInPixels = false,
 const GFont::XAlign                xalign  = GFont::XALIGN_CENTER,
 const GFont::YAlign                yalign  = GFont::YALIGN_CENTER);

DebugID debugDrawLabel
(const Point3&                      wsPos,
 const Vector3&                     csOffset,
 const String&                      text,
 const Color3&                      color,
 float                              displayTime = 0.0f,
 float                              size = 0.1f,
 bool                               sizeInPixels = false,
 const GFont::XAlign                xalign  = GFont::XALIGN_CENTER,
 const GFont::YAlign                yalign  = GFont::YALIGN_CENTER);

} // namespace G3D
