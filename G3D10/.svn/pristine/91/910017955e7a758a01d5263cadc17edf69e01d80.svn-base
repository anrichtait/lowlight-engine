/**
  \file G3D-base.lib/include/G3D-base/ParseError.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_ParseError_h

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/G3DString.h"

namespace G3D {

/** Thrown by TextInput, Any, and other parsers on unexpected input. */
class ParseError {
public:
    enum {UNKNOWN = -1};
    
    /** Empty means unknown. */
    String          filename;
    
    /** For a binary file, the location of the parse error. 
        ParseError::UNKNOWN if unknown. */
    int64           byte;

    /** For a text file, the line number is the line number of start of token which caused the
        exception.  1 is the first line of the file.  ParseError::UNKNOWN if unknown.  Note
        that you can use TextInput::Settings::startingLineNumberOffset to shift the effective
        line number that is reported by that class. */
    int             line;

    /** Character number (in the line) of the start of the token which caused the
        exception.  1 is the character in the line. ParseError::UNKNOWN if unknown. */
    int             character;

    String          message;
 
    ParseError() : byte(UNKNOWN), line(UNKNOWN), character(UNKNOWN) {}

    virtual ~ParseError() {}

    ParseError(const String& f, int l, int c, const String& m) :
        filename (f), byte(UNKNOWN), line(l), character(c), message(m) {}

    ParseError(const String& f, int64 b, const String& m) :
        filename (f), byte(b), line(UNKNOWN), character(UNKNOWN), message(m) {}

    /** If information is known, ends in ": ", otherwise empty */
    String formatFileInfo() const;
};

}
