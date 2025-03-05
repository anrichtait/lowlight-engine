/**
  \file G3D-gfx.lib/include/G3D-gfx/NSAutoreleasePoolWrapper.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef NSAUTORELEASEPOOLWRAPPER_H
#define NSAUTORELEASEPOOLWRAPPER_H

class NSApplicationWrapper {
public:
    NSApplicationWrapper();
    virtual ~NSApplicationWrapper();

private:
protected:
};

class NSAutoreleasePoolWrapper {
public:
    NSAutoreleasePoolWrapper();
    virtual ~NSAutoreleasePoolWrapper();

private:
    void* _pool;

protected:

};

#endif
