/**
  \file G3D-gfx.lib/source/initGLG3D.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-base/WeakCache.h"
#include "G3D-gfx/AudioDevice.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-app/UniversalMaterial.h"

namespace G3D {

class GFont;
class GuiTheme;

namespace _internal {
    WeakCache< String, shared_ptr<GFont> >& fontCache();
    WeakCache< String, shared_ptr<GuiTheme> >& themeCache();
    WeakCache<UniversalMaterial::Specification, shared_ptr<UniversalMaterial> >& materialCache();
}


static void GLG3DCleanupHook() {
    _internal::materialCache().clear();
    _internal::themeCache().clear();
    _internal::fontCache().clear();
    GLPixelTransferBuffer::deleteAllBuffers();

#   ifndef G3D_NO_FMOD
    delete AudioDevice::instance;
    AudioDevice::instance = nullptr;
#   endif
}


// Forward declaration.  See G3D.h
void initG3D(const G3DSpecification& spec = G3DSpecification());

void initGLG3D(const G3DSpecification& spec) {
    static bool initialized = false;
    if (! initialized) {
        initG3D(spec);
#       ifndef G3D_NO_FMOD
            AudioDevice::instance = new AudioDevice();
            AudioDevice::instance->init(spec.audio, 1000, spec.audioBufferLength, spec.audioNumBuffers);
#       endif
        atexit(&GLG3DCleanupHook);
        initialized = true;
    }
}

} // namespace
