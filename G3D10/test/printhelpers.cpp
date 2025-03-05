/**
  \file test/printhelpers.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "testassert.h"
#include "printhelpers.h"

void printLeader(const std::string& leader) {
    std::string clamped = leader.substr(0, 15);
    printf("%15s", clamped.c_str());
}
