/**
  \file G3D-base.lib/include/G3D-base/Journal.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_Journal_h
#define G3D_Journal_h

#include "G3D-base/platform.h"
#include "G3D-base/G3DString.h"

namespace G3D {

/** Routines for programmatically working with journal.md.html and journal.dox files.

  \sa G3D::GApp::Settings::ScreenCapture::outputDirectory, G3D::VideoRecordDialog, G3D::ScreenCapture, G3D::Log, G3D::System::findDataFile 
  */
class Journal {
private:

    // Currently an abstract class
    Journal() {}

public:

    /** Locates journal.dox or journal.md.html and returns the
        fully-qualified filename, starting a search from \a hint.
        Returns the empty string if not found. */
    static String findJournalFile(const String& hint = ".");

    /** Returns title of the first Doxygen "section" or Markdeep level
        1 header (underlined by === or prefixed by a single #) command
        the journal at \a journalFilename, or the empty string if no
        section title is found. Assumes that \a journalFilename
        exists. */
    static String firstSectionTitle(const String& journalFilename);

    /** Adds \a text to the bottom of the first section in the .dox or .md.html
        file at \a journalFilename. */
    static void appendToFirstSection(const String& journalFilename, const String& text);

    /** Inserts \a text immediately before the first "section" command
        in the .dox file or before the first level 1 header in the
        .md.html file at \a journalFilename. */
    static void insertNewSection(const String& journalFilename, const String& title, const String& text);

    static String formatImage(const String& journalFilename, const String& imageFilename, const String& caption, const String& description);
};

}

#endif
