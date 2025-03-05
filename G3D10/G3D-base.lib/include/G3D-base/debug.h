/**
  \file G3D-base.lib/include/G3D-base/debug.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_debug_h
#define G3D_debug_h

#include "G3D-base/platform.h"
#ifdef _MSC_VER
    #include <crtdbg.h>
#endif

#include "G3D-base/debugPrintf.h"
#include "G3D-base/debugAssert.h"

namespace G3D {

#ifdef _MSC_VER
    // Turn off 64-bit warnings
#   pragma warning(push)
#   pragma warning( disable : 4312)
#   pragma warning( disable : 4267)
#   pragma warning( disable : 4311)
#endif


/**
 Useful for debugging purposes.
 */
inline bool isValidHeapPointer(const void* x) {
    #ifdef _MSC_VER
        return 
            (x != nullptr) &&
            (x != (void*)0xcccccccc) && (x != (void*)0xdeadbeef) && (x != (void*)0xfeeefeee) &&
            (x != (void*)0xcdcdcdcd) && (x != (void*)0xabababab) && (x != (void*)0xfdfdfdfd);
    #else
        return x != nullptr;
    #endif
}

/**
 Returns true if the pointer is likely to be
 a valid pointer (instead of an arbitrary number). 
 Useful for debugging purposes.
 */
inline bool isValidPointer(const void* x) {
    #ifdef _MSC_VER
        return (x != nullptr) &&
            (x != (void*)0xcdcdcdcdcdcdcdcd) && (x != (void*)0xcccccccccccccccc) &&
            (x != (void*)0xcccccccc) && (x != (void*)0xdeadbeef) && (x != (void*)0xfeeefeee) &&
            (x != (void*)0xcdcdcdcd) && (x != (void*)0xabababab) && (x != (void*)0xfdfdfdfd);
    #else
        return x != nullptr;
    #endif
}

#ifdef _MSC_VER
#   pragma warning(pop)
#endif

}

#endif
