/**
  \file G3D-base.lib/source/G3DAllocator.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/G3DAllocator.h"
#include "G3D-base/System.h"

namespace G3D {
namespace _internal {

void* systemMalloc(size_t bytes) {
    return System::malloc(bytes);
}
void systemFree(void* p) {
    System::free(p);
}

} // namespace _internal
} // namespace G3D
