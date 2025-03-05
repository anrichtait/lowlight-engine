/**
  \file G3D-base.lib/include/G3D-base/DoNotInitialize.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef DoNotInitialize_h
#define DoNotInitialize_h

/** A special value that may be passed to overloaded constructors to 
    indicate that they should not be initialized.  Useful for optimizing
    code for classes like Matrix4 that might produce unnecessary 
    initialization in some contexts
  */
class DoNotInitialize {};

#endif
