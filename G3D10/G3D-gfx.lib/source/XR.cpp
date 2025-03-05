/**
  \file G3D-gfx.lib/source/XR.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-gfx/XR.h"

namespace G3D {

XR::Object::Object(int index, int nativeAPIIndex, const String& name) : m_name(name), m_index(index), m_nativeAPIIndex(nativeAPIIndex) {}


void XR::setFrame(const shared_ptr<XR::Object>& t, const CFrame& f, const CFrame& p) {
    t->m_frame = f;
    t->m_previousFrame = p;

    // Ensure that the matrices are well formed even if the underlying tracker
    // reported noisy data.
    t->m_frame.rotation.orthonormalize();
    t->m_previousFrame.rotation.orthonormalize();

    debugAssert(t->m_frame.rotation.isOrthonormal());
}


} // namespace G3D