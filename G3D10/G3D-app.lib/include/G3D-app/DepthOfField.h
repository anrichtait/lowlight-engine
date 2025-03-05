/**
  \file G3D-app.lib/include/G3D-app/DepthOfField.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_DepthOfField_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-gfx/Shader.h"
#include "G3D-app/DepthOfFieldSettings.h"

namespace G3D {

class RenderDevice;
class Camera;

/** 
  \brief Defocus post-process shader
  
  \cite Based on
  Bukowski, Hennessy, Osman, and McGuire, The Skylanders SWAP Force Depth-of-Field Shader, GPU Pro4, 
  175-184, A K Peters / CRC Press, April 26, 2013
 */
class DepthOfField : public ReferenceCountedObject {
public:
    
    enum DebugOption {
        NONE,
        SHOW_COC,
        SHOW_REGION,
        SHOW_NEAR,
        SHOW_BLURRY,
        SHOW_INPUT,
        SHOW_MID_AND_FAR,
        SHOW_SIGNED_COC,
    };

protected:

    String              m_debugName;

    DepthOfField(const String& debugName);

    /** Color in RGB, circle of confusion and 'near field' bit in A. 
        Precision determined by the input, RGB8, RGB16F, or RGB32F.

        The A channel values are always written with only 8 bits of
        effective precision.

        The radius (A channel) values are scaled and biased to [0, 1].
        Unpack them to pixel radii with:

        \code
        r = ((a * 2) - 1) * maxRadius
        \endcode

        Where maxRadius the larger of the maximum near and far field
        blurs.  The decoded radius is negative in the far field (the packed
        alpha channel should look like a head lamp on a dark night, with
        nearby objects bright, the focus field gray, and the distance black).
    */
    shared_ptr<Texture>                 m_packedBuffer;
    shared_ptr<Framebuffer>             m_packedFramebuffer;

    shared_ptr<Framebuffer>             m_horizontalFramebuffer;
    shared_ptr<Texture>                 m_tempNearBuffer;
    shared_ptr<Texture>                 m_tempBlurBuffer;

    shared_ptr<Framebuffer>             m_verticalFramebuffer;
    shared_ptr<Texture>                 m_nearBuffer;
    shared_ptr<Texture>                 m_blurBuffer;

    /** Allocates and resizes buffers */
    void resizeBuffers(shared_ptr<Texture> target, int reducedResolutionFactor, Vector2int16 trimBandThickness);

    /** Writes m_packedBuffer */
    void computeCoC
       (RenderDevice* rd, 
        const shared_ptr<Texture>&      color, 
        const shared_ptr<Texture>&      depth,
        const shared_ptr<Camera>&       camera,
        Vector2int16                    inputGuardBand,
        float&                          farRadiusRescale,
        float                           maxCoCRadiusPixels);

    void blurPass
       (RenderDevice*                   rd, 
        const shared_ptr<Texture>&      blurInput,
        const shared_ptr<Texture>&      nearInput,
        const shared_ptr<Framebuffer>&  output,
        bool                            horizontal, 
        const shared_ptr<Camera>&       camera,
        const Rect2D&                   fullViewport,
        float                           maxCoCRadiusPixels,
        bool                            diskFramebuffer);

    /** Writes to the currently-bound framebuffer. */
    void composite
       (RenderDevice*                   rd, 
        shared_ptr<Texture>             packedBuffer, 
        shared_ptr<Texture>             blurBuffer, 
        shared_ptr<Texture>             nearBuffer, 
        DebugOption                     d,
        Vector2int16                    trimBandThickness,
        float                           farRadiusRescale,
        bool                            diskFramebuffer);

public:

    /** \param debugName Used for naming textures. Does not affect which shaders are loaded.*/
    static shared_ptr<DepthOfField> create(const String& debugName = "G3D::DepthOfField");

    /** Applies depth of field blur to supplied images and renders to
        the currently-bound framebuffer.  The current framebuffer may
        have the \a color and \a depth values bound to it.

        Reads depth reconstruction and circle of confusion parameters
        from \a camera.

        Centers the output on the target framebuffer, so no explicit
        output guard band is specified.
    */
    virtual void apply
       (RenderDevice*             rd, 
        shared_ptr<Texture>       color, 
        shared_ptr<Texture>       depth, 
        const shared_ptr<Camera>& camera,
        Vector2int16              trimBandThickness,
        DebugOption               debugOption = NONE);
};

} // namespace G3D
