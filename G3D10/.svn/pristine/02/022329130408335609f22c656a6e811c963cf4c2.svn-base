/**
  \file G3D-base.lib/include/G3D-base/System.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef G3D_System_h
#define G3D_System_h

#include "G3D-base/platform.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/G3DGameUnits.h"
#include "G3D-base/BinaryFormat.h"
#include "G3D-base/FileNotFound.h"
#include "G3D-base/G3DString.h"
#include <memory>
#include <vector>


#ifdef G3D_OSX
#   define Zone OSX_Zone
#   define Fixed OSX_Fixed
#   define Point OSX_Point
#   include <CoreServices/CoreServices.h>
#   undef Point
#   undef Fixed
#   undef nil
#endif
namespace G3D {

    template<class T, size_t MIN_ELEMENTS> class Array;
    
/** G3D, SDL, and IJG libraries require license documentation
    to be distributed with your program.  This generates the
    string that must appear in your documentation. 
    <B>Your program can be commercial, closed-source</B> under
    any license you want.
    @deprecated Use System::license
*/
String license();

/**
@brief The order in which the bytes of an integer are stored on a
machine. 

Intel/AMD chips tend to be G3D_LITTLE_ENDIAN, Mac PPC's and Suns are
G3D_BIG_ENDIAN. However, this is primarily used to specify the byte
order of file formats, which are fixed.
*/
enum G3DEndian {
    G3D_BIG_ENDIAN, 
    G3D_LITTLE_ENDIAN
};

/**
 @brief OS and processor abstraction.  

 The first time any method is called the processor will be analyzed.
 Future calls are then fast.
 */
class System {
public:
    /**
       @param size Size of memory that the system was trying to allocate

       @param recoverable If true, the system will attempt to allocate again
       if the callback returns true.  If false, malloc is going to return 
       nullptr and this invocation is just to notify the application.

       @return Return true to force malloc to attempt allocation again if the
       error was recoverable.
     */
    typedef bool (*OutOfMemoryCallback)(size_t size, bool recoverable);

private:

    bool           m_initialized;
    bool           m_initializing;

    /** this holds the data directory set by the application (currently
        GApp) for use by findDataFile */
    String                m_appDataDir;
    std::vector<String>   m_appDataDirs;

    G3DEndian   m_machineEndian;
    String      m_cpuArch;
    String      m_operatingSystem;

    String      m_version;
    String      m_sourceControlRevision;

    OutOfMemoryCallback m_outOfMemoryCallback;

    /** @brief Used for the singleton instance only. */
    System();

    /** @brief The singleton instance. 

        Used instead of a global variable to ensure that the order of
        intialization is correct, which is critical because other
        globals may allocate memory using System::malloc.
     */
    static System& instance();

    void init();

public:
    
    /** atexit handling code invoked from G3DCleanupHook. */
    static void cleanup();

    /**
     Returns the endianness of this machine.
    */
    inline static G3DEndian machineEndian() {
        return instance().m_machineEndian;
    }

    /** e.g., "Windows", "GNU/Linux" */
    inline static const String& operatingSystem() {
        return instance().m_operatingSystem;
    }
    
    /** e.g., 80686 */
    inline static const String& cpuArchitecture() {
        return instance().m_cpuArch;
    }

    /**
       Returns the current date as a string in the form YYYY-MM-DD
    */
    static String currentDateString();

    /** Returns the current 24-hour local time as a string in the form HH:MM:SS */
    static String currentTimeString();

    /**
       Uses pooled storage to optimize small allocations (1 byte to 5
       kilobytes).  Can be 10x to 100x faster than calling \c malloc or
       \c new.
       
       The result must be freed with free.
       
       Threadsafe on Win32.
       
       @sa calloc realloc OutOfMemoryCallback free
    */
    static void* malloc(size_t bytes);
    
    static void* calloc(size_t n, size_t x);

    /**
     Version of realloc that works with System::malloc.
     */
    static void* realloc(void* block, size_t bytes);

    static void resetMallocPerformanceCounters();

    /** 
       Returns a string describing the current usage of the buffer pools used for
       optimizing System::malloc, and describing how well System::malloc is using
        its internal pooled storage.  "heap" memory was slow to
        allocate; the other data sizes are comparatively fast.
     */
    static String mallocStatus();

    /**
     Free data allocated with System::malloc.

     Threadsafe on Win32.
     */
    static void free(void* p);

    /**
       Guarantees that the start of the array is aligned to the 
       specified number of bytes.
    */
    static void* alignedMalloc(size_t bytes, size_t alignment);
    
    /**
     Frees memory allocated with alignedMalloc.
     */
    static void alignedFree(void* ptr);

    /** An implementation of memcpy that may be up to 2x as fast as the C library
        one on some processors.  Guaranteed to have the same behavior as memcpy
        in all cases. */
    static void memcpy(void* dst, const void* src, size_t numBytes);
    
    /** An implementation of memset that may be up to 2x as fast as the C library
        one on some processors.  Guaranteed to have the same behavior as memset
        in all cases. */
    static void memset(void* dst, uint8 value, size_t numBytes);
    
    /**
     Returns the fully qualified filename for the currently running executable.

     This is more reliable than arg[0], which may be intentionally set
     to an incorrect value by a calling program, relative to a now
     non-current directory, or obfuscated by sym-links.

     @cite Linux version written by Nicolai Haehnle <prefect_@gmx.net>, http://www.flipcode.com/cgi-bin/msg.cgi?showThread=COTD-getexename&forum=cotd&id=-1
     */
    static String currentProgramFilename();

    /** Name of this program. Note that you can mutate this string to
        set your app name explicitly.*/
    static String& appName();

    /** G3D Version string */
    inline static const String& version() {
        return instance().m_version;
    }

    /** Source control revision of G3D build. */
    static const String& g3dRevision() {
        return instance().m_sourceControlRevision;
    }

    /**
       @brief The optimization status of the G3D library (not the program compiled against it)

       Either "Debug" or "Release", depending on whether _DEBUG was
       defined at compile-time for the library.
      */
    static const String& build();

    /**
     Causes the current thread to yield for the specified duration
     and consume almost no CPU.
     The sleep will be extremely precise; it uses System::time() 
     to calibrate the exact yeild time.
     */
    static void sleep(RealTime t);

    /**
     Clears the console.
     Console programs only.
     */
    static void consoleClearScreen();

    /**
     Returns true if a key is waiting.
     Console programs only.
     */
    static bool consoleKeyPressed();
    
    /**
     Blocks until a key is read (use consoleKeyPressed to determine if
     a key is waiting to be read) then returns the character code for
     that key.
     */
    static int consoleReadKey();

    /**
     The actual time (measured in seconds since
     Jan 1 1970 midnight).
     
     Adjusted for local timezone and daylight savings
     time.   This is as accurate and fast as getCycleCount().
    */
    static RealTime time();

    inline static void setOutOfMemoryCallback(OutOfMemoryCallback c) {
        instance().m_outOfMemoryCallback = c;
    }

    /**
     When System::malloc fails to allocate memory because the system is
     out of memory, it invokes this handler (if it is not nullptr).
     The argument to the callback is the amount of memory that malloc
     was trying to allocate when it ran out.  If the callback returns
     true, System::malloc will attempt to allocate the memory again.
     If the callback returns false, then System::malloc will return nullptr.

     You can use outOfMemoryCallback to free data structures or to 
     register the failure.
     */
    inline static OutOfMemoryCallback outOfMemoryCallback() {
        return instance().m_outOfMemoryCallback;
    }

    /** Set an environment variable for the current process */
    static void setEnv(const String& name, const String& value);
    
    /** Get an environment variable for the current process.  Returns nullptr if the variable doesn't exist. */
    static const char* getEnv(const String& name);

    /**
     Prints a human-readable description of this machine
     to the text output stream.  Either argument may be nullptr.
     */
    static void describeSystem
       (class TextOutput& t);

    static void describeSystem
       (String&        s);

    /**
     Appends search paths specified by G3D10DATA environment
     variable to paths vector, since cannot rely on Array.
    */
    static void getG3DDataPaths(std::vector<String>& paths);

    /**
     Tries to locate the resource by looking in related directories.
     If found, returns the full path to the resource, otherwise
     returns the empty string.

     Looks in:

         - Literal interpretation of full (i.e., if it contains a fully-qualified name)
         - Last directory in which a file was found
         - Current directory
         - System::appDataDir (which is usually GApp::Settings.dataDir, which defaults to the directory containing the program binary)
         - System::appDataDir() + "data/"  (note that this may be a zipfile named "data" with no extension)
         - System::appDataDir() + "data.zip/"
         - ../data-files/ 
         - ../../data-files/
         - ../../../data-files/
         - $G3D10DATA directories. This must be a semicolon (windows) or colon (OS X/Linux) separated list of paths. It may be a single path.

       Plus the following subdirectories of those:

         - cubemap
         - gui
         - font
         - icon
         - models
         - image
         - sky
         - md2
         - md3
         - ifs
         - 3ds

        \param exceptionIfNotFound If true and the file is not found, throws G3D::FileNotFound.
     */    
    static String findDataFile(const String& full, bool exceptionIfNotFound = true, bool caseSensitive =
#ifdef G3D_WINDOWS
        false
#else
        true
#endif
        );

    static void initializeDirectoryArray(Array<String, 10>& directoryArray, bool caseSensitive =
#ifdef G3D_WINDOWS
        false
#else
        true
#endif
        );
    /**
        Sets the path that the application is using as its data directory.
        Used by findDataDir as an initial search location.  GApp sets this
        upon constrution.
    */
    static void setAppDataDir(const String& path);

    /**
    Sets additional paths that the application will search for data files.
    Used by findDataDir as an search locations.  GApp sets this
    upon constrution.
    */
    static void setAppDataDirs(const std::vector<String>& paths);

};


/** 
 \brief Implementation of a C++ Allocator (for example std::allocator) that uses G3D::System::malloc and G3D::System::free. 

 All instances of g3d_allocator are stateless.

 \sa G3D::MemoryManager, G3D::System::malloc, G3D::System::free
*/
template<class T>
class g3d_allocator {
public:
    /** Allocates n * sizeof(T) bytes of uninitialized storage by calling G3D::System::malloc() */
    [[nodiscard]] constexpr T* allocate(std::size_t n) {
        return static_cast<T*>(System::malloc(sizeof(T) * n));
    }

    /** Deallocates the storage referenced by the pointer p, which must be a pointer obtained by an earlier call to allocate() */
    constexpr void deallocate(T* p, std::size_t n) {
        System::free(p);
    }
};

} // namespace G3D

// https://en.cppreference.com/w/cpp/memory/allocator/operator_cmp
template< class T1, class T2 >
constexpr bool operator==( const G3D::g3d_allocator<T1>& lhs, const G3D::g3d_allocator<T2>& rhs ) noexcept {
    return true;
}

#ifdef G3D_OSX
#undef Zone
#endif

#endif
