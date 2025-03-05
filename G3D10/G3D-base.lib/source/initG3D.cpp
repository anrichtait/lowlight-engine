/**
  \file G3D-base.lib/source/initG3D.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include <stdlib.h>
#include "G3D-base/platform.h"
#include "G3D-base/System.h"
#include "G3D-base/Log.h"

namespace G3D {

namespace _internal {

    G3DSpecification& g3dInitializationSpecification() {
        static G3DSpecification s;
        return s;
    }

    void initializeNetwork();
    void cleanupNetwork();
}

static void G3DCleanupHook() {
    _internal::cleanupNetwork();
    System::cleanup();
}

void initG3D(const G3DSpecification& spec) {
    static bool initialized = false;
    
    if (! initialized) {
        initialized = true;
        Log::common(spec.logFilename);
        _internal::g3dInitializationSpecification() = spec;
        atexit(&G3DCleanupHook);
        
        _internal::initializeNetwork();
    }
}

} // namespace
