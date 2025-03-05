/**
  \file G3D-app.lib/source/TriTree.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/AABox.h"
#include "G3D-base/CollisionDetection.h"
#include "G3D-base/CubeMap.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/TriTree.h"
#include "G3D-app/Surface.h"
#include "G3D-app/UniversalSurfel.h"
#include "G3D-app/GBuffer.h"
#include "G3D-app/Camera.h"
#include "G3D-app/Scene.h"
#include "G3D-app/EmbreeTriTree.h"
#include "G3D-app/NativeTriTree.h"
#include "G3D-app/OptiXTriTree.h"
#include "G3D-app/VulkanTriTree.h"


namespace G3D {

shared_ptr<TriTree> TriTree::create(const shared_ptr<Scene>& scene, ImageStorage newImageStorage) {
    const shared_ptr<TriTree>& t = create((newImageStorage == ImageStorage::MOVE_TO_GPU) || (newImageStorage == ImageStorage::COPY_TO_GPU));
    t->setContents(scene, newImageStorage);
    return t;
}
    
shared_ptr<Surfel> TriTree::intersectRay(const Ray& ray) const {
    // See what the ray hits
    TriTree::Hit hit;
    if (intersectRay(ray, hit)) {
        // A triangle
        shared_ptr<Surfel> surfel;
        sample(hit, surfel);
        return surfel;
    } else {
        Radiance3 L_e;
        if (m_sky) {
            // The scene's skybox
            L_e = m_sky->bilinear(-ray.direction());
        } else {
            // No skybox. Assume all gray.
            L_e = Radiance3(0.2f);
        }
        return UniversalSurfel::createEmissive(L_e, ray.direction() * 1e6, -ray.direction());
    }
}

    
void TriTree::sample(const Hit& hit, shared_ptr<Surfel>& surfel) const {
    if (hit.triIndex != Hit::NONE) {
        m_triArray[hit.triIndex].sample(hit.u, hit.v, hit.triIndex, m_vertexArray, hit.backface, surfel);
    } else {
        surfel = nullptr;
    }
}


shared_ptr<TriTree> TriTree::create(bool gpuData, bool preferVulkan) {
#   if defined(G3D_X86) && (defined(G3D_WINDOWS) || defined(G3D_LINUX) || defined(G3D_MACOS)) 
        if (gpuData) {
#           ifdef G3D_WINDOWS
            if (preferVulkan) {
                // TODO: check for initialization in the same way OptiX does.
                return VulkanTriTree::create();
            }
            {
                static enum {UNINITIALIZED, YES, NO} hasOptiX = UNINITIALIZED;
                if (hasOptiX == UNINITIALIZED) {
                    const shared_ptr<OptiXTriTree>& test = OptiXTriTree::create();
                    if (test->supported()) {
                        hasOptiX = YES;
                        return test;
                    } else {
                        hasOptiX = NO;
                    }
                }

                if (hasOptiX == YES) {
                    return OptiXTriTree::create();
                } else {
                    return EmbreeTriTree::create();
                }
            }
#           else
            {
                return EmbreeTriTree::create();
            }
#           endif
        } else {
            return EmbreeTriTree::create();
        }
#   else
        // Android, ARM
        return NativeTriTree::create();
#   endif
}

} // G3D
