/**
  \file G3D-base.lib/source/Log.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Log.h"
#include "G3D-base/format.h"
#include "G3D-base/Array.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/FileSystem.h"
#include <time.h>

#ifdef G3D_WINDOWS
#   pragma warning(disable : 4091)
#   include <imagehlp.h>
#   pragma warning(default : 4091)
#else
#   include <stdarg.h>
#endif

namespace G3D {

void logPrintf(const char* fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    Log::common()->vprintf(fmt, arg_list);
    va_end(arg_list);
}


void logLazyPrintf(const char* fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    Log::common()->lazyvprintf(fmt, arg_list);
    va_end(arg_list);
}

Log* Log::commonLog = nullptr;

Log::Log(const String& filename) {
    this->filename = filename;

    logFile = FileSystem::fopen(filename.c_str(), "w");

    if (logFile == nullptr) {
        String drive, base, ext;
        Array<String> path;
        parseFilename(filename, drive, path, base, ext);
        String logName = base + ((ext != "") ? ("." + ext) : ""); 

        // Write time is greater than 1ms.  This may be a network drive.... try another file.
        #ifdef G3D_WINDOWS
            logName = String(std::getenv("TEMP")) + logName;
        #else
            logName = String("/tmp/") + logName;
        #endif

        logFile = FileSystem::fopen(logName.c_str(), "w");
    }

    // Use a large buffer (although we flush in logPrintf)
    setvbuf(logFile, nullptr, _IOFBF, 2048);

    fprintf(logFile, "Application Log\n");
    time_t t;
    time(&t);
    fprintf(logFile, "Start: %s\n", ctime(&t));
    fflush(logFile);

    if (commonLog == nullptr) {
        commonLog = this;
    }
}


Log::~Log() {
    section("Shutdown");
    println("Closing log file");
    
    // Make sure we don't leave a dangling pointer
    if (Log::commonLog == this) {
        Log::commonLog = nullptr;
    }

    if (logFile) {
        FileSystem::fclose(logFile);
    }
}


FILE* Log::getFile() const {
    return logFile;
}


Log* Log::common() {
    if (commonLog == nullptr) {
        commonLog = new Log();
    }
    return commonLog;
}


Log* Log::common(const String& filename) {
    if (! commonLog || commonLog->filename != filename) {
        alwaysAssertM(isNull(commonLog), "Common log already exists");
        commonLog = new Log(filename);
    }
    return commonLog;
}


String Log::getCommonLogFilename() {
    return common()->filename;
}


void Log::section(const String& s) {
    fprintf(logFile, "_____________________________________________________\n");
    fprintf(logFile, "\n    ###    %s    ###\n\n", s.c_str());
}


void Log::printf(const char* fmt, ...) {
    va_list arg_list;
    va_start(arg_list, fmt);
    print(vformat(fmt, arg_list));
    va_end(arg_list);
}


void Log::vprintf(const char* fmt, va_list argPtr) {
    vfprintf(logFile, fmt, argPtr);
    fflush(logFile);
}


void Log::lazyvprintf(const char* fmt, va_list argPtr) {
    vfprintf(logFile, fmt, argPtr);
}


void Log::print(const String& s) {
    fprintf(logFile, "%s", s.c_str());
    fflush(logFile);
}


void Log::println(const String& s) {
    fprintf(logFile, "%s\n", s.c_str());
    fflush(logFile);
}

}
