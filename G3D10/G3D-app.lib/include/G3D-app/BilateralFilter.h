/**
  \file G3D-app.lib/include/G3D-app/BilateralFilter.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
/**
\file BilateralFilter.h

\maintainer, Michael Mara

\created 2016-05-08
\edited  2016-10-28

*/
#include "G3D-base/platform.h"
#include "G3D-base/Vector2.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-app/BilateralFilterSettings.h"

namespace G3D {

class Framebuffer;
class RenderDevice;
class Texture;
class GBuffer;

/**
(Separated) 2D Bilateral Filter.

Although, bilateral filters are not mathematically separable,
doing so gives a good approximation in exchange for reducing
quadratic to linear run time and so is a common practice.

\sa BilateralFilterSettings
*/
class BilateralFilter : public ReferenceCountedObject {
protected:

    mutable shared_ptr<Framebuffer> m_intermediateFramebuffer;

    void apply1D(RenderDevice* rd, const shared_ptr<Texture>& source, const shared_ptr<GBuffer>& gbuffer, const Vector2& direction, const BilateralFilterSettings& settings) const;

public:

    BilateralFilter() {}

    static shared_ptr<BilateralFilter> create() {
        return createShared<BilateralFilter>();
    }

    /**
        Applies a Bilateral Filter. Handles intermediate storage in
        a texture of the same format as the source texture.

        Assumes that the following fields are available:
        
        - G3D::GBuffer::Field::CS_NORMAL (if normal weight or plane weight is nonzero)
        - G3D::GBuffer::Field::DEPTH_AND_STENCIL (if depth weight or plane weight is nonzero)
        - G3D::GBuffer::Field::GLOSSY (if glossy weight is nonzero)
    */
    void apply(RenderDevice* rd, const shared_ptr<Texture>& source, const shared_ptr<Framebuffer>& destination, const shared_ptr<GBuffer>& gbuffer, const BilateralFilterSettings& settings) const;

};
}


