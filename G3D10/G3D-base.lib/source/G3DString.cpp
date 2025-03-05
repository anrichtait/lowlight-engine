/**
  \file G3D-base.lib/source/G3DString.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/G3DString.h"
#include "G3D-base/g3dmath.h"

#ifdef G3D_LINUX
// etext is defined by the linker as the end of the text segment
// edata is defined by the linker as the end of the initialized data segment
// the edata section comes after the etext segment
extern char etext, edata;
#endif

namespace G3D {

#ifdef G3D_WINDOWS
 #define abs _abs64
#endif

bool inConstSegment(const char* c) {
#ifdef G3D_LINUX
    // We can't use __builtin_constant(c) because that checks whether the expression
    // is a constexpr, not where the address is. Here, the expression is never constant
    // (it is a runtime value).

    // check if the address is in the initialized data section (edata)
    return (c > &etext) && (c < &edata);
#else
    static const char* testStr = "__G3DString.cpp__";
    static const uintptr_t PROBED_CONST_SEG_ADDR = uintptr_t(testStr);

    // MSVC assigns the const_seg to very high addresses that are grouped together.
    // Beware that when using explicit SEGMENT https://docs.microsoft.com/en-us/cpp/assembler/masm/segment?view=vs-2017
    // or const_seg https://docs.microsoft.com/en-us/cpp/preprocessor/const-seg?view=vs-2017, this does not work.
    // However, that simply disables some optimizations in G3DString, it doesn't break it.
    //
    // Assume it is at least 5 MB long.
    return (abs(int64(uintptr_t(c) - PROBED_CONST_SEG_ADDR)) < int64(5000000));
#endif
}

}
