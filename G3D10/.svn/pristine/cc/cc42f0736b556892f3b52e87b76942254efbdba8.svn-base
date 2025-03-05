/**
  \file G3D-gfx.lib/source/Milestone.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-gfx/Milestone.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/GLCaps.h"

namespace G3D {


Milestone::Milestone(const String& n) : m_glSync(GL_NONE), m_name(n) {
    m_glSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}


Milestone::~Milestone() {
    glDeleteSync(m_glSync);
    m_glSync = GL_NONE;
}


GLenum Milestone::_wait(float timeout) const {
    const uint64 t = iFloor(min(timeout, 100000.0f) * 1000);
    GLenum result = glClientWaitSync(m_glSync, 0, t);
    debugAssertM(result != GL_WAIT_FAILED, "glClientWaitSync failed");

    return result;
}


bool Milestone::completed() const {
    GLenum e = _wait(0);
    return (e == GL_ALREADY_SIGNALED) || (e == GL_CONDITION_SATISFIED);
}


bool Milestone::wait(float timeout) {
    return (_wait(timeout) != GL_TIMEOUT_EXPIRED);
}

} // namespace G3D
