/**
  \file G3D-base.lib/include/G3D-base/DepthFirstTreeBuilder.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once

#include "G3D-base/platform.h"

namespace G3D {

/**
 Template class for a depth first tree traversal supporting
 methods enterChild and goToParent.
 */

template <class Node>
class DepthFirstTreeBuilder { 
public: 
    DepthFirstTreeBuilder() {}

    virtual ~DepthFirstTreeBuilder() {}

    /** Add child to current node and go down a level. */
    virtual void enterChild(const Node& node) = 0; 

    /** Go up a level, to parent of current node. */
    virtual void goToParent() = 0;
};

} // namespace
