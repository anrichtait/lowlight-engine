/**
  \file G3D-base.lib/include/G3D-base/FileNotFound.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_FileNotFound_h
#define G3D_FileNotFound_h

#include "G3D-base/platform.h"
#include "G3D-base/G3DString.h"

namespace G3D {

/** Thrown by various file opening routines if the file is not found. 

   \sa ParseError, System::findDataFile
*/
class FileNotFound {
public:
    String filename;
    String message;

    FileNotFound() {}
    FileNotFound(const String& f, const String& m) : filename(f), message(m) {}
    virtual ~FileNotFound(){};
};

} // G3D

#endif
