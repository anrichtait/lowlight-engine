/**
  \file G3D-base.lib/source/enumclass.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/enumclass.h"
#include <string.h>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#   define strtok_r strtok_s
#endif

namespace G3D {
namespace _internal {

const char** smartEnumParseNames(const char* enumValList) {
    char *saveptr = nullptr;

    // We'll intentionally mutate (via strtok) and then leak the memory allocated here so that it
    // can be referenced by a static local variable inside the enum class toString method.  We 
    // could register an atexit handler to clear the allocated memory for all enums at once on shutdown
    // if this bothered us.
    char* copy = strdup(enumValList);
    std::vector<char*> ptrs;

    const char* DELIMITERS = " ,";

    // Tokenize
    for (char* token = strtok_r(copy, DELIMITERS, &saveptr); token; token = strtok_r(nullptr, DELIMITERS, &saveptr)) {
        ptrs.push_back(token);
    }

    // Allocate one extra element for the nullptr terminator.  Note that array will also be intentionally
    // leaked.
    char** array = (char**)::malloc(sizeof(char*) * (ptrs.size() + 1));
    ::memcpy(array, &ptrs[0], sizeof(char*) * ptrs.size());
    array[ptrs.size()] = nullptr;
    return (const char**)array;
}

}
}
