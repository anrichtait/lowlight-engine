/**
  \file G3D-app.lib/include/G3D-app/GaussianBlur.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_GAUSSIANBLUR_H
#define G3D_GAUSSIANBLUR_H

#include "G3D-base/Table.h"
#include "G3D-base/Vector2.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Shader.h"

namespace G3D {

/**
 1D Gaussian blur.  Call twice to produce a 2D blur.

 Operates on the graphics card; this requires a RenderDevice.  See G3D::gaussian for
 gaussian filter coefficients on the CPU.
 */
class GaussianBlur {
public:

    enum Direction {VERTICAL, HORIZONTAL};

    /** 
     Blurs the source to the current G3D::Framebuffer.  Assumes RenderDevice::push2D rendering mode is already set.
     Blurs the alpha channel the same as any color channel, however, you must have alphaWrite
     enabled to obtain that result.

     2D blur is not directly supported because handling of the intermediate texture is different
     for Framebuffer and backbuffer rendering.

     @param N Number of taps in the filter (filter kernel width)
     @param direction Direction of the blur.  For best results, use Vector2(1,0) and Vector(0,1).
     @param destSize output dimensions
     \param clear Clear the target first?
     \param unitArea If true, the taps sum to 1.  If false, the center tap has magnitude 1.
     \param computeFraction If >= 0, only compute on a disk that covers this fraction of the diameter of the screen (max of width, height).
    */
    static void apply(class RenderDevice* rd, const shared_ptr<Texture>& source, const Vector2& direction, int N, const Vector2& destSize, bool clear = true, bool unitArea = true, float stddevMultiplier = 1.0f, float computeFraction = -1.0f);
    static void apply(class RenderDevice* rd, const shared_ptr<Texture>& source, const Vector2& direction = Vector2(1.0f, 0.0f), int N = 17);

    /** Returns the preamble for the GuassianBlur shader containing:
        \code
        #define KERNEL_RADIUS <number>
        float gaussCoef[KERNEL_RADIUS];
        \endcode
        */
    static String getPreamble(int N, bool unitArea, float stddevMultiplier = 1.0f);
};

}

#endif

