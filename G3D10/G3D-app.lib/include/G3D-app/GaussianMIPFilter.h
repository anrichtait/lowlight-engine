/**
  \file G3D-app.lib/include/G3D-app/GaussianMIPFilter.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2020, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#include "G3D-base/ReferenceCount.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/Framebuffer.h"
#include "G3D-base/CoordinateFrame.h"

namespace G3D {

    class GaussianMIPFilter : public ReferenceCountedObject {
    protected:

        shared_ptr<Texture>                     m_normalZ;
        shared_ptr<Framebuffer>                 m_framebuffer;

        /** Clamp the mip level to make the reflections look good.
            Because this is a gaussian blur, we need this clamp to
            avoid the sky sampling a high mip level and overdarkening
            when blurred with the relatively dark ceiling. */
        const int                               m_mipLimit = 3;

        GaussianMIPFilter();

    public:

        static shared_ptr<GaussianMIPFilter> create();
        int mipLimit() const {
            return m_mipLimit;
        }
        void apply
        (RenderDevice* rd,
            const shared_ptr<Texture>& texture,
            const shared_ptr<Texture>& positionMIP0,
            const shared_ptr<Texture>& normalMIP0,
            const shared_ptr<Texture>& glossy,
            const CFrame& cameraFrame) const;
    };
}
