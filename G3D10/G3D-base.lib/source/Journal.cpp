/**
  \file G3D-base.lib/source/Journal.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/Journal.h"
#include "G3D-base/debugAssert.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/TextInput.h"
#include "G3D-base/enumclass.h"
#include <regex>

namespace G3D {

G3D_DECLARE_ENUM_CLASS(JournalSyntax, DOXYGEN, MARKDEEP);
    
static JournalSyntax detectSyntax(const String& journalFilename) {
    return endsWith(toLower(journalFilename), ".dox") ? JournalSyntax::DOXYGEN : JournalSyntax::MARKDEEP;
}


String Journal::findJournalFile(const String& hint) {
    Array<String> searchPaths;
    
    if (endsWith(hint, ".dox")) {
        searchPaths.append(FilePath::parent(hint));
    } else {
        searchPaths.append(hint);
    } 
    
    searchPaths.append(
        FileSystem::currentDirectory(), 
        FilePath::concat(FileSystem::currentDirectory(), ".."),
        FilePath::concat(FileSystem::currentDirectory(), "../journal"),
        FilePath::concat(FileSystem::currentDirectory(), "../../journal"),
        FilePath::concat(FileSystem::currentDirectory(), "../../../journal"));
    searchPaths.append(
        FilePath::concat(FileSystem::currentDirectory(), "../journal2"),
        FilePath::concat(FileSystem::currentDirectory(), "../../journal2"),
        FilePath::concat(FileSystem::currentDirectory(), "../../../journal2"));
    
    const Array<String> filenames = { "journal.md.html","journal2.md.html", "journal3.md.html","journal.dox" };

    for (int i = 0; i < searchPaths.length(); ++i) {    
        for (int j = 0; j < filenames.length(); ++j) {
            const String& t = FilePath::concat(searchPaths[i], filenames[j]);
            if (FileSystem::exists(t)) {
                return t;
            }
        }
    }

    return "";
}


static const std::regex& headerRegex() {
    // Find either "\n# <stuff>\n\n" or "\n<stuff>\n===...\n"
    // With newlines able to be either unix-style "\n" or windows style "\r\n".
#   define NEWLINE "\r?\n"
    static const std::regex HEADER_REGEX(NEWLINE "#[^\n\r]+" NEWLINE NEWLINE "|" NEWLINE "[^\r\n#][^\r\n]*" NEWLINE "={3,}\\s*" NEWLINE, std::regex::ECMAScript);
#   undef NEWLINE
    return HEADER_REGEX;
}


static size_t findSection(JournalSyntax syntax, const String& fileContents, size_t start = 0) {
    if (syntax == JournalSyntax::DOXYGEN) {
        // Search for the section title
        size_t pos = fileContents.find("\\section", start);
        const size_t pos2 = fileContents.find("@section", start);
        
        if (pos == String::npos) {
            pos = pos2;
        } else if (pos2 != String::npos) {
            pos = min(pos, pos2);
        }
        
        return (int)pos;
    } else {
        
        std::cmatch match;
        if (std::regex_search(fileContents.c_str(), match, headerRegex())) {
            // Advance past the starting newline for the match to get to actual section
            if (fileContents.at(match.position()) == '\r') {
                return match.position() + 2;
            }
            return match.position() + 1;
        } else {
            return String::npos;
        }
    }
}


String Journal::firstSectionTitle(const String& journalFilename) {
    alwaysAssertM(FileSystem::exists(journalFilename), journalFilename + " not found.");

    const JournalSyntax syntax = detectSyntax(journalFilename);

    const String& file = readWholeFile(journalFilename);

    // Search for the section title
    size_t pos = findSection(syntax, file);

    if (pos == String::npos) {
        // No section found
        return "";
    } else if (syntax == JournalSyntax::DOXYGEN) {
        pos += String("@section").length();

        // Read that line
        size_t end = file.find("\n", pos);
        if (end == String::npos) {
            // Back up by the necessary end-comment character
            end = file.length() - 2;
        }

        const String& sectionStatement = file.substr(pos, end - pos + 1);

        try {
            TextInput parser(TextInput::FROM_STRING, sectionStatement);
            // Skip the label
            parser.readSymbol();

            return trimWhitespace(parser.readUntilNewlineAsString());
        } catch (...) {
            // Journal parsing failed
            return "";
        }
    } else {
        // Get just title (<stuff>) from header match: "\n# <stuff>\n\n" or "\n<stuff>\n===...\n"
        size_t end = file.find("\n", pos);
        debugAssert(end != String::npos);

        // Find the position before \r\n not just \n so we don't return \r as part of the title
        if (file.at(end - 1) == '\r') {
            end = end - 1;
        }

        // The first character is either # from ATX or not for STL format then copy to newline
        return trimWhitespace(file.substr((file[pos] == '#') ? pos + 1 : pos, end - pos));
    }
}


void Journal::appendToFirstSection(const String& journalFilename, const String& text) {
    alwaysAssertM(FileSystem::exists(journalFilename), journalFilename + " not found.");

    const JournalSyntax syntax = detectSyntax(journalFilename);
    const String& file = readWholeFile(journalFilename);

    // Search for the section title
    size_t pos = findSection(syntax, file);

    String combined;
    if (syntax == JournalSyntax::DOXYGEN) {
        if (pos != String::npos) {
            // Skip to the end of the section line
            pos = file.find('\n', pos);
            if (pos != String::npos) {
                // Go *past* that character
                ++pos;
            }
        } 
        
        if (pos == String::npos) {
            // No section found: look for the end of the documentation comment
            pos = file.find("*/");
            if (pos == String::npos) {
                pos = file.length() - 1;
            }
        }

    } else {
        // Get past the header
        if (pos != String::npos) {
            const bool isATX = (file[pos] == '#');

            pos = file.find('\n', pos);
            if (pos == String::npos) {
                pos = file.length() - 1;
            }

            if (! isATX) {
                // Jump over the === signs as well by looking for the NEXT newline
                pos = file.find('\n', pos + 1);
                if (pos == String::npos) {
                    pos = file.length() - 1;
                }
            }
        }
    }


    combined = file.substr(0, pos) + text + "\n" + file.substr(pos);

    writeWholeFile(journalFilename, combined);
}


void Journal::insertNewSection(const String& journalFilename, const String& title, const String& text) {
    alwaysAssertM(FileSystem::exists(journalFilename), journalFilename + " not found.");

    const JournalSyntax syntax = detectSyntax(journalFilename);
    const String& file = readWholeFile(journalFilename);

    // Search for the section title
    size_t pos = findSection(syntax, file);

    if (pos == String::npos) {
        // No section found: look for the beginning
        if (syntax == JournalSyntax::DOXYGEN) {
            pos = file.find("/*");
            if (pos == String::npos) {
                pos = 0;
            } else {
                pos += 2;
            }
        } else {
            // Skip over any opening meta tag and title
            pos = file.find("<meta");
            if (pos != String::npos) {
                // Jump past it:
                pos = file.find('>', pos) + 1;
            } else {
                pos = 0;
            }
            
            while ((pos < file.length() - 1) && isWhitespace(file[pos])) {
                ++pos;
            }

            // TODO: Jump past any indented lines as well
        }
    }

    String section;
    if (syntax == JournalSyntax::DOXYGEN) {
        time_t t1;
        ::time(&t1);
        tm* t = localtime(&t1);

        const String& sectionName = format("S%4d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
        section = "\\section " + sectionName + " " + title + "\n\n" + text + "\n";
    } else {
        section = "\n" + trimWhitespace(title) + "\n=============================================================\n" + text + "\n";
    }
    
    const String& combined = file.substr(0, pos) + section + "\n" + file.substr(pos);
    writeWholeFile(journalFilename, combined);
}


static String escapeDoxygenCaption(const String& s) {
    String r;
    for (size_t i = 0; i < s.length(); ++i) {
        const char c = s[i];
        if ((c == ',') || (c == '}') || (c == '{') || (c == '\"')) {
            // Escape these characters
            r += '\\';
        }
        r += c;
    }
    return r;
}


String Journal::formatImage(const String& journalFilename, const String& imageFilename, const String& caption, const String& description) {
    const bool isVideo = endsWith(toLower(imageFilename), ".mp4");

    const JournalSyntax syntax = detectSyntax(journalFilename);

    const String& journalFullPath = FileSystem::resolve(journalFilename);
    const String& imageFullPath = FileSystem::resolve(imageFilename);

    const String& prefix = G3D::greatestCommonPrefix(journalFullPath, imageFullPath);
    const String& relativeImagePath = imageFullPath.substr(prefix.length());

    if (syntax == JournalSyntax::DOXYGEN) {
        const String macroString = isVideo ? "video" : "thumbnail";
        return "\n\\" + macroString + "{" + relativeImagePath + ", " + escapeDoxygenCaption(caption) + "}\n\n" + description + "\n";
    } else {
        const String& discussionSection = (description.empty()) ? "" : "\n\n" + description;
        return "\n![" + caption + "](" + relativeImagePath + ")" + discussionSection + "\n";
    }
}


} // G3D
