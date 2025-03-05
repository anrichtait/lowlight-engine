/**
  \file G3D-app.lib/include/G3D-app/TriTreeBase.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include <functional>
#include "G3D-base/platform.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/Ray.h"
#include "G3D-base/Array.h"
#include "G3D-gfx/CPUVertexArray.h"
#include "G3D-app/Tri.h"
#include "G3D-app/TriTree.h"
#ifndef _MSC_VER
#include <stdint.h>
#endif

namespace G3D {
class Surface;
class Surfel;
class Material;
class AABox;
class GBuffer;

/** Common partial implementation base class for ray-casting data structures. */
class TriTreeBase : public TriTree {
protected:

    static void copyToCPU
       (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
        const shared_ptr<GLPixelTransferBuffer>& rayDirection,
        Array<Ray>&                              rayBuffer,
		const int width = -1,
		const int height = -1);

    static void copyToCPU
       (const shared_ptr<GLPixelTransferBuffer>& rayCoherence,
        Array<float>&                            rayCoherenceBuffer);

public:
    
    virtual ~TriTreeBase();

    virtual void clear() override;

    virtual void setContents
        (const Array<shared_ptr<Surface>>&        surfaceArray, 
         ImageStorage                             newImageStorage = ImageStorage::COPY_TO_CPU) override;
                                                  
    virtual void setContents                      
       (const Array<Tri>&                         triArray, 
        const CPUVertexArray&                     vertexArray,
        ImageStorage                              newStorage = ImageStorage::COPY_TO_CPU) override;
                                                  
    virtual void setContents                      
       (const shared_ptr<class Scene>&            scene, 
        ImageStorage                              newStorage = ImageStorage::COPY_TO_CPU) override;
                                                  
    virtual void intersectRays                    
        (const Array<Ray>&                        rays,
         Array<Hit>&                              results,
         IntersectRayOptions                      options         = IntersectRayOptions(0)) const override;

    virtual void intersectRays
        (const Array<Ray>&                        rays,
         Array<shared_ptr<Surfel>>&               results,
         IntersectRayOptions                      options         = IntersectRayOptions(0),
         const Array<float>&                      coneBuffer = Array<float>()) const override;

    /** \deprecated */
    virtual void intersectRays
        (const shared_ptr<Texture>&               rayOrigin,
         const shared_ptr<Texture>&               rayDirection,
         const shared_ptr<GBuffer>&               results,
         IntersectRayOptions                      options         = IntersectRayOptions(0),
         const shared_ptr<Texture>&               rayCone         = nullptr) const override;

    virtual void intersectRays
        (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
         const shared_ptr<GLPixelTransferBuffer>& rayDirection,
         const shared_ptr<GLPixelTransferBuffer>  results[5],
         IntersectRayOptions                      options         = IntersectRayOptions(0),
         const shared_ptr<GLPixelTransferBuffer>& rayCone         = nullptr,
         const int baseMipLevel = 0,
		 const Vector2int32 wavefrontDimensions = Vector2int32(-1,-1),
		 const RenderMask mask = 0xFF) const override;

    /** \deprecated */
    virtual void intersectRays
        (const shared_ptr<Texture>&               rayOrigin,
         const shared_ptr<Texture>&               rayDirection,
         const shared_ptr<Texture>&               booleanResults,
         IntersectRayOptions                      options         = IntersectRayOptions(0)) const override;

    virtual void intersectRays
        (const shared_ptr<GLPixelTransferBuffer>& rayOrigin,
         const shared_ptr<GLPixelTransferBuffer>& rayDirection,
         const shared_ptr<GLPixelTransferBuffer>& booleanResults,
         IntersectRayOptions                      options         = IntersectRayOptions(0)) const override;

    virtual void intersectRays
        (const Array<Ray>&                        rays,
         Array<bool>&                             results,
         IntersectRayOptions                      options         = IntersectRayOptions(0)) const override;

    virtual void intersectBox
        (const AABox&                             box,
         Array<Tri>&                              results) const override;

    virtual void intersectSphere
        (const Sphere&                            sphere,
         Array<Tri>&                              triArray) const override;
};

} // G3D
