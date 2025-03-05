/**
  \file G3D-gfx.lib/include/G3D-gfx/Milestone.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_gfx_Milestone_h
#define G3D_gfx_Milestone_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/Array.h"
#include "G3D-gfx/glheaders.h"

namespace G3D {

/**
 Used to set a marker in the GPU instruction stream that
 the CPU can later query <i>or</i> block on.

 Abstracts the OpenGL ARB_sync/OpenGL 3.2 API 
 (http://www.opengl.org/registry/specs/ARB/sync.txt)
 that replaced the older NV_fence (http://oss.sgi.com/projects/ogl-sample/registry/NV/fence.txt) API.

 G3D names this class 'milestone' instead of 'fence' 
 to clarify that it is not a barrier--the CPU and GPU can
 both pass a milestone without blocking unless the CPU explicitly
 requests a block.
 */
class Milestone : public ReferenceCountedObject {
private:

    friend class RenderDevice;
    
    GLsync                  m_glSync;
    String             m_name;
            
    // The methods are private because in the future
    // we may want to move parts of the implementation
    // into RenderDevice.

    Milestone(const String& n);
    
    /** Wait for it to be reached. 
    \returnglClientWaitSync result */
    GLenum _wait(float timeout) const;

public:
    /**
    Create a new Milestone (GL fence) and insert it into the GPU execution queue.
    This must be called on a thread that has the current OpenGL Context */
    static shared_ptr<Milestone> create(const String& name) {
        return  shared_ptr<Milestone>(new Milestone(name));
    }

    ~Milestone();

    inline const String& name() const {
        return m_name;
    }

    /**
     Stalls the CPU until the GPU has finished the milestone.

     \return false if timed out, true if the milestone completed.

     \sa  completed
     */
    bool wait(float timeout = finf());

    /** Returns true if this milestone has completed execution.
        \sa  wait */
    bool completed() const;
};


}

#endif
