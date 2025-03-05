/**
  \file G3D-base.lib/source/System.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/System.h"
#include "G3D-base/debug.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/TextOutput.h"
#include "G3D-base/G3DGameUnits.h"
#include "G3D-base/Crypto.h"
#include "G3D-base/prompt.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/Log.h"
#include "G3D-base/Table.h"
#include "G3D-base/Thread.h"
#include "G3D-base/units.h"
#include "G3D-base/FileSystem.h"
#include <chrono>
#include <time.h>

// Uncomment the following line to turn off G3D::System memory
// allocation and use the operating system's malloc.
//#define NO_BUFFERPOOL

#include <cstdlib>

#ifdef G3D_WINDOWS

#   include <conio.h>
#   include <sys/timeb.h>
#   include "G3D-base/RegistryUtil.h"
#include <Ole2.h>

#elif defined(G3D_LINUX) 

#   include <stdlib.h>
#   include <stdio.h>
#   include <errno.h>
#   include <sys/types.h>
#   include <sys/select.h>
#   include <termios.h>
#   include <unistd.h>
#   include <sys/ioctl.h>
#   include <sys/time.h>
#   include <pthread.h>

#elif defined(G3D_OSX)

    #include <stdlib.h>
    #include <stdio.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <sys/select.h>
    #include <sys/time.h>
    #include <termios.h>
    #include <unistd.h>
    #include <pthread.h>
    #include <mach-o/arch.h>

    #include <sstream>
    #include <CoreServices/CoreServices.h>
#endif

#ifdef G3D_X86
// SIMM include
#include <xmmintrin.h>
#endif


namespace G3D {
    
/** Called from init */
static void getG3DVersion(String& s);
static void getSourceControlRevision(String& revision);

/** Called from init */
static G3DEndian checkEndian();

void* System_malloc(size_t s) {
    return System::malloc(s);
}


void System_free(void* p) {
    System::free(p); 
}


System& System::instance() {
    static System thesystem;
    return thesystem;
}


System::System() :
    m_initialized(false),
    m_initializing(false),
    m_machineEndian(G3D_LITTLE_ENDIAN),
    m_cpuArch("Uninitialized"),
    m_operatingSystem("Uninitialized"),
    m_version("Uninitialized"),
    m_outOfMemoryCallback(nullptr) {
    init();
}


void System::init() {
    // NOTE: Cannot use most G3D data structures or utility functions
    // in here because they are not initialized.

    if (m_initialized) {
        return;
    } else {
        alwaysAssertM(!m_initializing, "Attempting to call System::init() while in System::init()");
        m_initializing = true;
    }

    // Build G3D version string
    getG3DVersion(m_version);

    // Get library source revision
    getSourceControlRevision(m_sourceControlRevision);
    
    m_machineEndian = checkEndian();

#ifdef __arm64__
    m_cpuArch = "ARM 64";
#else
    m_cpuArch = "Intel/AMD x64";
#endif

    // Get the operating system name (also happens to read some other information)
#    ifdef G3D_WINDOWS
        OSVERSIONINFO osVersionInfo;
        osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        const bool success = GetVersionEx(&osVersionInfo) != 0;

        if (success) {
            char c[1000];
            sprintf(c, "Windows %d.%d build %d Platform %d %ls",
                osVersionInfo.dwMajorVersion,
                osVersionInfo.dwMinorVersion,
                osVersionInfo.dwBuildNumber,
                osVersionInfo.dwPlatformId,
                osVersionInfo.szCSDVersion);
            m_operatingSystem = c;
        } else {
            m_operatingSystem = "Windows";
        }
    
#    elif defined(G3D_LINUX) || defined(G3D_FREEBSD)

        {
            // Find the operating system using the 'uname' command
            FILE* f = popen("uname -a", "r");

            int len = 100;
            char* r = (char*)::malloc(len * sizeof(char));
            (void)fgets(r, len, f);
            // Remove trailing newline
            if (r[strlen(r) - 1] == '\n') {
                r[strlen(r) - 1] = '\0';
            }
            pclose(f);
            f = nullptr;

            m_operatingSystem = r;
            ::free(r);
        }

#   elif defined(G3D_OSX)

        // Operating System:
        SInt32 macVersion;
        Gestalt(gestaltSystemVersion, &macVersion);

        int major = (macVersion >> 8) & 0xFF;
        int minor = (macVersion >> 4) & 0xF;
        int revision = macVersion & 0xF;

        {
            char c[1000];
            sprintf(c, "OS X %x.%x.%x", major, minor, revision); 
            m_operatingSystem = c;
        }
#   endif

    m_appDataDir = FilePath::parent(System::currentProgramFilename());

    // set initialized at the end to ensure no calls to init() from init()
    m_initialized = true;
    m_initializing = false;
}


void getG3DVersion(String& s) {

    const char* debug =
#       ifdef G3D_DEBUG
            " (Debug)";
#       else
            "";
#       endif

    char cstr[100];
    if ((G3D_VER % 100) != 0) {
        sprintf(cstr, "G3D Innovation Engine %d.%02d beta %d%s",
                G3D_VER / 10000,
                (G3D_VER / 100) % 100,
                G3D_VER % 100,
                debug);
    } else {
        sprintf(cstr, "G3D Innovation Engine %d.%02d%s",
                G3D_VER / 10000,
                (G3D_VER / 100) % 100,
                debug);
    }

    s = cstr;
}

void getSourceControlRevision(String& revision) {
    #include "generated/svnrevision.h"
    revision = __g3dSvnRevision;
}

// Places where specific files were most recently found.  This is
// used to cache seeking of common files.
static Table<String, String> lastFound;

// Places to look in findDataFile
static Array<String> directoryArray;

void System::initializeDirectoryArray(Array<String>& directoryArray, bool caseSensitive) {
    const String initialAppDataDir(instance().m_appDataDir);
    // Initialize the directory array
    RealTime t0 = System::time();

    std::vector<String> baseDirArray;
        
    baseDirArray.push_back(FileSystem::currentDirectory());

    if (! initialAppDataDir.empty()) {
        baseDirArray.push_back(initialAppDataDir);
        baseDirArray.push_back(FilePath::concat(initialAppDataDir, "data"));
        baseDirArray.push_back(FilePath::concat(initialAppDataDir, "data.zip"));
    } else {
        baseDirArray.push_back("data");
        baseDirArray.push_back("data.zip");
    }

    for (const String& path : instance().m_appDataDirs) {
        baseDirArray.push_back(path);
    }

    getG3DDataPaths(baseDirArray);

    static const String subdirs[] = 
        {"font", "gui", "shader", "model", "cubemap", "icon", "material", "image", "md2", "md3", "ifs", "3ds", "sky", "music", "sound", "scene", "voxel", ""};
    for (int j = 0; j < int(baseDirArray.size()); ++j) {
        const String& d = baseDirArray[j];
//logPrintf("%s", d.c_str());
        if (d.empty() || FileSystem::exists(d)) {
//logPrintf(" exists\n");
            directoryArray.append(d);
            for (int i = 0; ! subdirs[i].empty(); ++i) {
                const String& p = FilePath::concat(d, subdirs[i]);
                if (FileSystem::exists(p, true, caseSensitive)) {
                    directoryArray.append(p);
                }
            }
        } else {
//logPrintf(" does not exist\n");
        }
    }
    logLazyPrintf("Initializing System::findDataFile took %fs\n", System::time() - t0);
}

void System::getG3DDataPaths(std::vector<String>& paths) {
    instance().init();

    if (! _internal::g3dInitializationSpecification().deployMode) {
        const char* c = System::getEnv("G3D10DATA");
        const String& fullG3D10DATA = isNull(c) ? "" : c;
        const char splitChar = 
    #           ifdef G3D_WINDOWS
                ';'
    #           else
                ':'
    #           endif
                ;

        const Array<String>& allG3D10DATAPaths = stringSplit(fullG3D10DATA, splitChar);
        for (int i = 0; i < allG3D10DATAPaths.size(); ++i) {
              if (! allG3D10DATAPaths[i].empty()) {
                  paths.push_back(FileSystem::resolve(allG3D10DATAPaths[i]));
              }
        }
    }
}

#define MARK_LOG()
//#define MARK_LOG() logPrintf("%s(%d)\n", __FILE__, __LINE__)
String System::findDataFile
(const String&       _full,
 bool                errorIfNotFound,
 bool                caseSensitive) {
MARK_LOG();

    const String& full = FilePath::expandEnvironmentVariables(_full);

    // First check if the file exists as requested.  This will go
    // through the FileSystemCache, so most calls do not touch disk.
    if (FileSystem::exists(full, true, caseSensitive)) {
        return full;
    }

MARK_LOG();
    // Now check where we previously found this file.
    String* last = lastFound.getPointer(full);
    if (last != nullptr) {
        if (FileSystem::exists(*last, true, caseSensitive)) {
            // Even if cwd has changed the file is still present.
            // We won't notice if it has been deleted, however.
            return *last;
        } else {
            // Remove this from the cache it is invalid
            lastFound.remove(full);
        }
    }

MARK_LOG();

    const String initialAppDataDir(instance().m_appDataDir);
    if (directoryArray.size() == 0) {
        initializeDirectoryArray(directoryArray, caseSensitive);
    }

MARK_LOG();

    for (int i = 0; i < directoryArray.size(); ++i) {
        const String& p = FilePath::concat(directoryArray[i], full);
        if (FileSystem::exists(p, true, caseSensitive)) {
            const String& s = FilePath::canonicalize(p);
            lastFound.set(full, s);
            return s;
        }
    }
MARK_LOG();

    if (errorIfNotFound) {
        // Generate an error message.  Delay this operation until we know that we need it;
        // otherwise all of the string concatenation would run on each successful find.
        String locations;
        for (int i = 0; i < directoryArray.size(); ++i) {
            locations += "\'" + FilePath::concat(directoryArray[i], full) + "'\n";
        }
MARK_LOG();

        String msg = "Could not find '" + full + "'.\n\n";
        msg +=     "  cwd                    = '" + FileSystem::currentDirectory() + "'\n";
        if (System::getEnv("G3D10DATA")) {
            msg += "  G3D10DATA               = '" + String(System::getEnv("G3D10DATA")) + "'";
            msg += "\n";
        } else {
            msg += "  G3D10DATA               = (environment variable is not defined)\n";
        }
MARK_LOG();
        msg +=     "  GApp::Settings.dataDir = '" + initialAppDataDir + "'";
        if (! FileSystem::exists(initialAppDataDir, true, caseSensitive)) {
            msg += " (illegal path!)";
        }
        msg += "\n";

        msg += "\nFilenames tested:\n" + locations;

MARK_LOG();
logPrintf("%s\n", msg.c_str());
        throw FileNotFound(full, msg);
        alwaysAssertM(false, msg);
    }
MARK_LOG();

    // Not found
    return "";
}
#undef MARK_LOG

void System::setAppDataDir(const String& path) {
    instance().m_appDataDir = path;

    // Wipe the findDataFile cache
    lastFound.clear();
    directoryArray.clear();
}

void System::setAppDataDirs(const std::vector<String>& paths) {
    // Intentional copy
    instance().m_appDataDirs = paths;

    // Wipe the findDataFile cache
    lastFound.clear();
    directoryArray.clear();
}


void System::cleanup() {
    lastFound.clear();
    directoryArray.clear();
}


const String& System::build() {
    const static String b =
#   ifdef _DEBUG
        "Debug";
#   else 
        "Release";
#   endif

    return b;
}


static G3DEndian checkEndian() {
    int32 a = 1;
    if (*(uint8*)&a == 1) {
        return G3D_LITTLE_ENDIAN;
    } else {
        return G3D_BIG_ENDIAN;
    }
}


void System::memcpy(void* dst, const void* src, size_t numBytes) {
    ::memcpy(dst, src, numBytes);
}

void System::memset(void* dst, uint8 value, size_t numBytes) {
    ::memset(dst, value, numBytes);
}


/** Removes the 'd' that icompile / Morgan's VC convention appends. */
static String computeAppName(const String& start) {
    if (start.size() < 2) {
        return start;
    }

    if (start[start.size() - 1] == 'd') {
        // Maybe remove the 'd'; see if ../ or ../../ has the same name
        char tmp[1024];
        (void)getcwd(tmp, sizeof(tmp));
        String drive, base, ext;
        Array<String> path;
        parseFilename(tmp, drive, path, base, ext);

        String shortName = start.substr(0, start.size() - 1);

        if ((path.size() > 1) && (toLower(path.last()) == toLower(shortName))) {
            return shortName;
        }

        if ((path.size() > 2) && (toLower(path[path.size() - 2]) == toLower(shortName))) {
            return shortName;
        }
    }

    return start;
}


String& System::appName() {
    static String n = computeAppName(filenameBase(currentProgramFilename()));
    return n;
}


String System::currentProgramFilename() {
    char filename[2048];

#   ifdef G3D_WINDOWS
    {
        GetModuleFileNameA(nullptr, filename, sizeof(filename));
    } 
#   elif defined(G3D_OSX)
    {
        // Run the 'ps' program to extract the program name
        // from the process ID.
        int pid;
        FILE* fd;
        char cmd[80];
        pid = getpid();
        sprintf(cmd, "ps -p %d -o comm=\"\"", pid);

        fd = popen(cmd, "r");
        int s = fread(filename, 1, sizeof(filename), fd);
        // filename will contain a newline.  Overwrite it:
        filename[s - 1] = '\0';
        pclose(fd);
    }
#   else
    {
        int ret = readlink("/proc/self/exe", filename, sizeof(filename));
            
        // In case of an error, leave the handling up to the caller
        if (ret == -1) {
            return "";
        }
            
        debugAssert((int)sizeof(filename) > ret);
            
        // Ensure proper nullptr termination
        filename[ret] = 0;      
    }
    #endif

    return filename;
}


void System::sleep(RealTime t) {
    alwaysAssertM((t < inf()) && ! isNaN(t), "Cannot sleep for infinite time");

    // Overhead of calling this function, measured from a previous run.
    static const RealTime OVERHEAD = 0.00006f;

    RealTime now = time();
    RealTime wakeupTime = now + t - OVERHEAD;

    RealTime remainingTime = wakeupTime - now;
    RealTime sleepTime = 0;

    // On Windows, a "time slice" is measured in quanta of 3-5 ms (http://support.microsoft.com/kb/259025)
    // Sleep(0) yields the remainder of the time slice, which could be a long time.
    // A 1 ms minimum time experimentally kept the "Empty GApp" at nearly no CPU load at 100 fps,
    // yet nailed the frame timing perfectly.
    static RealTime minRealSleepTime = 3 * units::milliseconds();

    while (remainingTime > 0) {

        if (remainingTime > minRealSleepTime * 2.5) {
            // Safe to use Sleep with a time... sleep for half the remaining time
            sleepTime = max(remainingTime * 0.5, 0.0005);
        } else if (remainingTime > minRealSleepTime) {
            // Safe to use Sleep with a zero time;
            // causes the program to yield only
            // the current time slice, and then return.
            sleepTime = 0;
        } else {
            // Not safe to use Sleep; busy wait
            sleepTime = -1;
        }

        if (sleepTime >= 0) {
            #ifdef G3D_WINDOWS
                // Translate to milliseconds
                Sleep((int)(sleepTime * 1e3));
            #else
                // Translate to microseconds
                usleep((int)(sleepTime * 1e6));
            #endif
        }

        now = time();
        remainingTime = wakeupTime - now;
    }
}


void System::consoleClearScreen() {
#   ifdef G3D_WINDOWS
        (void)system("cls");
#   else
        (void)system("clear");
#   endif
}


bool System::consoleKeyPressed() {
    #ifdef G3D_WINDOWS
    
        return _kbhit() != 0;

    #else
    
        static const int STDIN = 0;
        static bool initialized = false;

        if (! initialized) {
            // Use termios to turn off line buffering
            termios term;
            tcgetattr(STDIN, &term);
            term.c_lflag &= ~ICANON;
            tcsetattr(STDIN, TCSANOW, &term);
            setbuf(stdin, nullptr);
            initialized = true;
        }

        #ifdef G3D_LINUX

            int bytesWaiting;
            ioctl(STDIN, FIONREAD, &bytesWaiting);
            return bytesWaiting;

        #else

            timeval timeout;
            fd_set rdset;

            FD_ZERO(&rdset);
            FD_SET(STDIN, &rdset);
            timeout.tv_sec  = 0;
            timeout.tv_usec = 0;

            return select(STDIN + 1, &rdset, nullptr, nullptr, &timeout);
        #endif
    #endif
}


int System::consoleReadKey() {
#   ifdef G3D_WINDOWS
        return _getch();
#   else
        char c;
        (void)read(0, &c, 1);
        return c;
#   endif
}


RealTime System::time() {
    using namespace std::chrono;
    return duration_cast<duration<double>>(system_clock::now().time_since_epoch()).count();
}


////////////////////////////////////////////////////////////////
#define ALIGNMENT_SIZE 16 // must be at least sizeof(size_t)

#define REALPTR_TO_USERPTR(x)   ((uint8*)(x) + ALIGNMENT_SIZE)
#define USERPTR_TO_REALPTR(x)   ((uint8*)(x) - ALIGNMENT_SIZE)
#define USERSIZE_TO_REALSIZE(x)       ((x) + ALIGNMENT_SIZE)
#define REALSIZE_FROM_USERPTR(u) (*(size_t*)USERPTR_TO_REALPTR(ptr) + ALIGNMENT_SIZE)
#define USERSIZE_FROM_USERPTR(u) (*(size_t*)USERPTR_TO_REALPTR(ptr))

class BufferPool {
public:

    /** Only store buffers up to these sizes (in bytes) in each pool->
        Different pools have different management strategies.

        A large block is preallocated for tiny buffers; they are used with
        tremendous frequency.  Other buffers are allocated as demanded.
        Tiny buffers are 128 bytes long because that seems to align well with
        cache sizes on many machines.
      */
    enum {tinyBufferSize = 256, smallBufferSize = 2048, medBufferSize = 8192};

    /** 
       Most buffers we're allowed to store.
       250000 * { 128 |  256} = {32 | 64} MB (preallocated)
        40000 * {1024 | 2048} = {40 | 80} MB (allocated on demand)
         5000 * {4096 | 8192} = {20 | 40} MB (allocated on demand)
     */
    enum {maxTinyBuffers = 250000, maxSmallBuffers = 40000, maxMedBuffers = 5000};

private:

    /** Pointer given to the program.  Unless in the tiny heap, the user size of the block is stored right in front of the pointer as a uint32.*/
    typedef void* UserPtr;

    /** Actual block allocated on the heap */
    typedef void* RealPtr;

    class MemBlock {
    public:
        UserPtr     ptr;
        size_t      bytes;

        inline MemBlock() : ptr(nullptr), bytes(0) {}
        inline MemBlock(UserPtr p, size_t b) : ptr(p), bytes(b) {}
    };

    MemBlock smallPool[maxSmallBuffers];
    int smallPoolSize;

    MemBlock medPool[maxMedBuffers];
    int medPoolSize;

    /** The tiny pool is a single block of storage into which all tiny
        objects are allocated.  This provides better locality for
        small objects and avoids the search time, since all tiny
        blocks are exactly the same size. */
    void* tinyPool[maxTinyBuffers];
    int tinyPoolSize;

    /** Pointer to the data in the tiny pool */
    void* tinyHeap;

    Spinlock            m_lock;

    inline void lock() {
        m_lock.lock();
    }

    inline void unlock() {
        m_lock.unlock();
    }

    /** 
     Malloc out of the tiny heap. Returns nullptr if allocation failed.
     */
    inline UserPtr tinyMalloc(size_t bytes) {
        // Note that we ignore the actual byte size
        // and create a constant size block.
        (void)bytes;
        assert(tinyBufferSize >= bytes);

        UserPtr ptr = nullptr;

        if (tinyPoolSize > 0) {
            --tinyPoolSize;

            // Return the old last pointer from the freelist
            ptr = tinyPool[tinyPoolSize];

#           ifdef G3D_DEBUG
                if (tinyPoolSize > 0) {
                    assert(tinyPool[tinyPoolSize - 1] != ptr); 
                     //   "System::malloc heap corruption detected: "
                     //   "the last two pointers on the freelist are identical (during tinyMalloc).");
                }
#           endif

            // nullptr out the entry to help detect corruption
            tinyPool[tinyPoolSize] = nullptr;
        }

        return ptr;
    }

    /** Returns true if this is a pointer into the tiny heap. */
    bool inTinyHeap(UserPtr ptr) {
        return 
            (ptr >= tinyHeap) && 
            (ptr < (uint8*)tinyHeap + maxTinyBuffers * tinyBufferSize);
    }

    void tinyFree(UserPtr ptr) {
        assert(ptr);
        assert(tinyPoolSize < maxTinyBuffers);
 //           "Tried to free a tiny pool buffer when the tiny pool freelist is full.");

#       ifdef G3D_DEBUG
            if (tinyPoolSize > 0) {
                UserPtr prevOnHeap = tinyPool[tinyPoolSize - 1];
                assert(prevOnHeap != ptr); 
//                    "System::malloc heap corruption detected: "
//                    "the last two pointers on the freelist are identical (during tinyFree).");
            }
#       endif

        assert(tinyPool[tinyPoolSize] == nullptr);

        // Put the pointer back into the free list
        tinyPool[tinyPoolSize] = ptr;
        ++tinyPoolSize;

    }

    void flushPool(MemBlock* pool, int& poolSize) {
        for (int i = 0; i < poolSize; ++i) {
            bytesAllocated -= USERSIZE_TO_REALSIZE(pool[i].bytes);
            ::free(USERPTR_TO_REALPTR(pool[i].ptr));
            pool[i].ptr = nullptr;
            pool[i].bytes = 0;
        }
        poolSize = 0;
    }


    /** Allocate out of a specific pool.  Return nullptr if no suitable 
        memory was found. */
    UserPtr poolMalloc(MemBlock* pool, int& poolSize, const int maxPoolSize, size_t bytes) {

        // OPT: find the smallest block that satisfies the request.

        // See if there's something we can use in the buffer pool.
        // Search backwards since usually we'll re-use the last one.
        for (int i = (int)poolSize - 1; i >= 0; --i) {
            if (pool[i].bytes >= bytes) {
                // We found a suitable entry in the pool.

                // No need to offset the pointer; it is already offset
                UserPtr ptr = pool[i].ptr;

                // Remove this element from the pool, replacing it with
                // the one from the end (same as Array::fastRemove)
                --poolSize;
                pool[i] = pool[poolSize];

                return ptr;
            }
        }

        if (poolSize == maxPoolSize) {
            // Free even-indexed pools, and compact array in the same loop
            for (int i = 0; i < poolSize; i += 2) {
                bytesAllocated -= USERSIZE_TO_REALSIZE(pool[i].bytes);
                ::free(USERPTR_TO_REALPTR(pool[i].ptr));
                pool[i].ptr = nullptr;
                pool[i].bytes = 0;
                // Compact: (i/2) is the next open slot
                pool[i/2].ptr   = pool[i+1].ptr;
                pool[i/2].bytes = pool[i+1].bytes;
                pool[i+1].ptr = nullptr;
                pool[i+1].bytes = 0;
            }
            poolSize = poolSize/2;
            if (maxPoolSize == maxMedBuffers) {
                ++medPoolPurgeCount;
            } else if (maxPoolSize == maxSmallBuffers) {
                ++smallPoolPurgeCount;
            }

        }

        return nullptr;
    }

public:

    /** Count of memory allocations that have occurred. */
    int totalMallocs;
    int mallocsFromTinyPool;
    int mallocsFromSmallPool;
    int mallocsFromMedPool;

    int smallPoolPurgeCount;
    int medPoolPurgeCount;

    /** Amount of memory currently allocated (according to the application). 
        This does not count the memory still remaining in the buffer pool,
        but does count extra memory required for rounding off to the size
        of a buffer.
        Primarily useful for detecting leaks.*/
    std::atomic_size_t bytesAllocated;

    BufferPool() {
        totalMallocs         = 0;

        mallocsFromTinyPool  = 0;
        mallocsFromSmallPool = 0;
        mallocsFromMedPool   = 0;

        bytesAllocated       = 0;

        tinyPoolSize         = 0;
        tinyHeap             = nullptr;

        smallPoolSize        = 0;

        medPoolSize          = 0;

        smallPoolPurgeCount = 0;
        medPoolPurgeCount   = 0;


        // Initialize the tiny heap as a bunch of pointers into one
        // pre-allocated buffer.
        tinyHeap = ::malloc(maxTinyBuffers * tinyBufferSize);
        for (int i = 0; i < maxTinyBuffers; ++i) {
            tinyPool[i] = (uint8*)tinyHeap + (tinyBufferSize * i);
        }
        tinyPoolSize = maxTinyBuffers;
    }


    ~BufferPool() {
        ::free(tinyHeap);
        flushPool(smallPool, smallPoolSize);
        flushPool(medPool, medPoolSize);
    }

    
    UserPtr realloc(UserPtr ptr, size_t bytes) {
        if (ptr == nullptr) {
            return malloc(bytes);
        }

        if (inTinyHeap(ptr)) {
            if (bytes <= tinyBufferSize) {
                // The old pointer actually had enough space.
                return ptr;
            } else {
                // Free the old pointer and malloc
                
                UserPtr newPtr = malloc(bytes);
                System::memcpy(newPtr, ptr, tinyBufferSize);
                lock();
                tinyFree(ptr);
                unlock();
                return newPtr;

            }
        } else {
            // In one of our heaps.

            // See how big the block really was
            size_t userSize = USERSIZE_FROM_USERPTR(ptr);
            if (bytes <= userSize) {
                // The old block was big enough.
                return ptr;
            }

            // Need to reallocate and move
            UserPtr newPtr = malloc(bytes);
            System::memcpy(newPtr, ptr, userSize);
            free(ptr);
            return newPtr;
        }
    }


    UserPtr malloc(size_t bytes) {
        lock();
        ++totalMallocs;

        if (bytes <= tinyBufferSize) {

            UserPtr ptr = tinyMalloc(bytes);

            if (ptr) {
                debugAssertM((intptr_t)ptr % 16 == 0, "BufferPool::tinyMalloc returned non-16 byte aligned memory");
                ++mallocsFromTinyPool;
                unlock();
                return ptr;
            }

        } 
        
        // Failure to allocate a tiny buffer is allowed to flow
        // through to a small buffer
        if (bytes <= smallBufferSize) {
            
            UserPtr ptr = poolMalloc(smallPool, smallPoolSize, maxSmallBuffers, bytes);

            if (ptr) {
                debugAssertM((intptr_t)ptr % 16 == 0, "BufferPool::poolMalloc(small) returned non-16 byte aligned memory");
                ++mallocsFromSmallPool;
                unlock();
                return ptr;
            }

        } else if (bytes <= medBufferSize) {
            // Note that a small allocation failure does *not* fall
            // through into a medium allocation because that would
            // waste the medium buffer's resources.

            UserPtr ptr = poolMalloc(medPool, medPoolSize, maxMedBuffers, bytes);

            if (ptr) {
                debugAssertM((intptr_t)ptr % 16 == 0, "BufferPool::poolMalloc(med) returned non-16 byte aligned memory");
                ++mallocsFromMedPool;
                unlock();
                debugAssertM(ptr != nullptr, "BufferPool::malloc returned nullptr");
                return ptr;
            }
        }

        bytesAllocated.fetch_add(USERSIZE_TO_REALSIZE(bytes));
        unlock();

        // Heap allocate

        // Allocate 4 extra bytes for our size header (unfortunate,
        // since malloc already added its own header).
        RealPtr ptr = ::malloc(USERSIZE_TO_REALSIZE(bytes));
        if (ptr == nullptr) {
#           ifdef G3D_WINDOWS
                // Check for memory corruption
                alwaysAssertM(_CrtCheckMemory() == TRUE, "Heap corruption detected.");
#           endif

            // Flush memory pools to try and recover space
            flushPool(smallPool, smallPoolSize);
            flushPool(medPool, medPoolSize);
            ptr = ::malloc(USERSIZE_TO_REALSIZE(bytes));
        }

        if (ptr == nullptr) {
            if ((System::outOfMemoryCallback() != nullptr) &&
                (System::outOfMemoryCallback()(USERSIZE_TO_REALSIZE(bytes), true) == true)) {
                // Re-attempt the malloc
                ptr = ::malloc(USERSIZE_TO_REALSIZE(bytes));
                
            }
        }

        if (ptr == nullptr) {
            if (System::outOfMemoryCallback() != nullptr) {
                // Notify the application
                System::outOfMemoryCallback()(USERSIZE_TO_REALSIZE(bytes), false);
            }
#           ifdef G3D_DEBUG
            debugPrintf("::malloc(%d) returned nullptr\n", (int)USERSIZE_TO_REALSIZE(bytes));
#           endif
            debugAssertM(ptr != nullptr, 
                         "::malloc returned nullptr. Either the "
                         "operating system is out of memory or the "
                         "heap is corrupt.");
            return nullptr;
        }

        ((size_t*)ptr)[0] = bytes;
        debugAssertM((intptr_t)REALPTR_TO_USERPTR(ptr) % 16 == 0, "::malloc returned non-16 byte aligned memory");
        return REALPTR_TO_USERPTR(ptr);
    }


    void free(UserPtr ptr) {
        if (ptr == nullptr) {
            // Free does nothing on null pointers
            return;
        }

        assert(isValidPointer(ptr));

        if (inTinyHeap(ptr)) {
            lock();
            tinyFree(ptr);
            unlock();
            return;
        }

        size_t bytes = USERSIZE_FROM_USERPTR(ptr);

        lock();
        if (bytes <= smallBufferSize) {
            if (smallPoolSize < maxSmallBuffers) {
                smallPool[smallPoolSize] = MemBlock(ptr, bytes);
                ++smallPoolSize;
                unlock();
                return;
            }
        } else if (bytes <= medBufferSize) {
            if (medPoolSize < maxMedBuffers) {
                medPool[medPoolSize] = MemBlock(ptr, bytes);
                ++medPoolSize;
                unlock();
                return;
            }
        }
        bytesAllocated.fetch_sub(USERSIZE_TO_REALSIZE(bytes));
        unlock();

        // Free; the buffer pools are full or this is too big to store.
        ::free(USERPTR_TO_REALPTR(ptr));
    }

    String mallocRatioString() const {
        if (totalMallocs > 0) {
            int pooled = mallocsFromTinyPool +
                         mallocsFromSmallPool + 
                         mallocsFromMedPool;

            int total = totalMallocs;

            return format("Percent of Mallocs: %5.1f%% <= %db, %5.1f%% <= %db, "
                          "%5.1f%% <= %db, %5.1f%% > %db",
                          100.0 * mallocsFromTinyPool  / total,
                          BufferPool::tinyBufferSize,
                          100.0 * mallocsFromSmallPool / total,
                          BufferPool::smallBufferSize,
                          100.0 * mallocsFromMedPool   / total,
                          BufferPool::medBufferSize,
                          100.0 * (1.0 - (double)pooled / total),
                          BufferPool::medBufferSize);
        } else {
            return "No System::malloc calls made yet.";
        }
    }

    String status() const {
        String tinyPoolString = format("Tiny Pool: %5.1f%% of %d x %db Free", 100.0 * tinyPoolSize / maxTinyBuffers, 
                                       maxTinyBuffers, tinyBufferSize);
        String poolSizeString = format("Pool Sizes: %5d/%d x %db, %5d/%d x %db, %5d/%d x %db",
                                       tinyPoolSize,     maxTinyBuffers,     tinyBufferSize, 
                                       smallPoolSize,    maxSmallBuffers,    smallBufferSize,
                                       medPoolSize,      maxMedBuffers,      medBufferSize);

        int pooled = mallocsFromTinyPool +
            mallocsFromSmallPool + 
            mallocsFromMedPool;
        int outOfPoolsMallocs = totalMallocs - pooled;
        String outOfBufferMemoryString = format("Total out of pools mallocs: %d; Bytes allocated: %d", outOfPoolsMallocs, int(bytesAllocated));
        String purgeString = format("Small Pool Purges: %d; Med Pool Purges: %d", smallPoolPurgeCount, medPoolPurgeCount);
        return mallocRatioString() + "\n" + poolSizeString + "\n" + outOfBufferMemoryString + "\n" + purgeString;

    }
};

// Dynamically allocated because we need to ensure that
// the buffer pool is still around when the last global variable 
// is deallocated.
static BufferPool* bufferpool = nullptr;

String System::mallocStatus() {    
#ifndef NO_BUFFERPOOL
    return bufferpool->status();
#else
    return "NO_BUFFERPOOL";
#endif
}


void System::resetMallocPerformanceCounters() {
#ifndef NO_BUFFERPOOL
    bufferpool->totalMallocs         = 0;
    bufferpool->mallocsFromMedPool   = 0;
    bufferpool->mallocsFromSmallPool = 0;
    bufferpool->mallocsFromTinyPool  = 0;
#endif
}


#ifndef NO_BUFFERPOOL
inline void initMem() {
    // Putting the test here ensures that the system is always
    // initialized, even when globals are being allocated.
    static bool initialized = false;
    if (! initialized) {
        bufferpool = new BufferPool();
        initialized = true;
    }
}
#endif


void* System::malloc(size_t bytes) {
#ifndef NO_BUFFERPOOL
    initMem();
    return bufferpool->malloc(bytes);
#else
    return ::malloc(bytes);
#endif
}

void* System::calloc(size_t n, size_t x) {
#ifndef NO_BUFFERPOOL
    void* b = System::malloc(n * x);
    debugAssertM(b != nullptr, "System::malloc returned nullptr");
    debugAssertM(isValidHeapPointer(b), "System::malloc returned an invalid pointer");
    System::memset(b, 0, n * x);
    return b;
#else
    return ::calloc(n, x);
#endif
}


void* System::realloc(void* block, size_t bytes) {
#ifndef NO_BUFFERPOOL
    initMem();
    return bufferpool->realloc(block, bytes);
#else
    return ::realloc(block, bytes);
#endif
}


void System::free(void* p) {
#ifndef NO_BUFFERPOOL
    bufferpool->free(p);
#else
    return ::free(p);
#endif
}


void* System::alignedMalloc(size_t bytes, size_t alignment) {

    alwaysAssertM(isPow2((uint32)alignment), "alignment must be a power of 2");

    // We must align to at least a word boundary.
    alignment = max(alignment, sizeof(void *));

    // Pad the allocation size with the alignment size and the size of
    // the redirect pointer.  This is the worst-case size we'll need.
    // Since the alignment size is at least teh word size, we don't
    // need to allocate space for the redirect pointer.  We repeat the max here
    // for clarity.
    size_t totalBytes = bytes + max(alignment, sizeof(void*));

    size_t truePtr = (size_t)System::malloc(totalBytes);

    if (truePtr == 0) {
        // malloc returned nullptr
        return nullptr;
    }

    debugAssert(isValidHeapPointer((void*)truePtr));
    #ifdef G3D_WINDOWS
    // The blocks we return will not be valid Win32 debug heap
    // pointers because they are offset 
    //  debugAssert(_CrtIsValidPointer((void*)truePtr, totalBytes, TRUE) );
    #endif


    // We want alignedPtr % alignment == 0, which we'll compute with a
    // binary AND because 2^n - 1 has the form 1111... in binary.
    const size_t bitMask = (alignment - 1);

    // The return pointer will be the next aligned location that is at
    // least sizeof(void*) after the true pointer. We need the padding
    // to have a place to write the redirect pointer.
    size_t alignedPtr = truePtr + sizeof(void*);

    const size_t remainder = alignedPtr & bitMask;
    
    // Add what we need to make it to the next alignment boundary, but
    // if the remainder was zero, let it wrap to zero and don't add
    // anything.
    alignedPtr += ((alignment - remainder) & bitMask);

    debugAssert((alignedPtr & bitMask) == 0);
    debugAssert((alignedPtr - truePtr + bytes) <= totalBytes);

    // Immediately before the aligned location, write the true array location
    // so that we can free it correctly.
    size_t* redirectPtr = (size_t *)(alignedPtr - sizeof(void *));
    redirectPtr[0] = truePtr;

    debugAssert(isValidHeapPointer((void*)truePtr));

    #if defined(G3D_WINDOWS) && defined(G3D_DEBUG)
        if (bytes < 0xFFFFFFFF) { 
            debugAssert( _CrtIsValidPointer((void*)alignedPtr, (int)bytes, TRUE) );
        }
    #endif
    return (void *)alignedPtr;
}


void System::alignedFree(void* _ptr) {
    if (_ptr == nullptr) {
        return;
    }

    size_t alignedPtr = (size_t)_ptr;

    // Back up one word from the pointer the user passed in.
    // We now have a pointer to a pointer to the true start
    // of the memory block.
    size_t* redirectPtr = (size_t*)(alignedPtr - sizeof(void *));

    // Dereference that pointer so that ptr = true start
    void* truePtr = (void*)redirectPtr[0];

    debugAssert(isValidHeapPointer((void*)truePtr));
    System::free(truePtr);
}


void System::setEnv(const String& name, const String& value) {
    String cmd = name + "=" + value;
#   ifdef G3D_WINDOWS
        _putenv(cmd.c_str());
#   else
        // Many linux implementations of putenv expect char*
        putenv(const_cast<char*>(cmd.c_str()));
#   endif
}


const char* System::getEnv(const String& name) {
    return getenv(name.c_str());
}


static void var(TextOutput& t, const String& name, const String& val) {
    t.writeSymbols(name, "=");
    t.writeString(val);
    t.writeNewline();
}


static void var(TextOutput& t, const String& name, const bool val) {
    t.writeSymbols(name, "=", val ? "Yes" : "No");
    t.writeNewline();
}


static void var(TextOutput& t, const String& name, const int val) {
    t.writeSymbols(name, "=");
    t.writeNumber(val);
    t.writeNewline();
}


void System::describeSystem(
    String&        s) {

    TextOutput t;
    describeSystem(t);
    t.commitString(s);
}

void System::describeSystem(
    TextOutput& t) {

    t.writeSymbols("App", "{");
    t.writeNewline();
    t.pushIndent();
    {
        var(t, "Name", System::currentProgramFilename());
        char cwd[1024];
        (void)getcwd(cwd, 1024);
        var(t, "cwd", String(cwd));
    }
    t.popIndent();
    t.writeSymbols("}");
    t.writeNewline();
    t.writeNewline();

    t.writeSymbols("OS", "{");
    t.writeNewline();
    t.pushIndent();
    {
        var(t, "Name", System::operatingSystem());
    }
    t.popIndent();
    t.writeSymbols("}");
    t.writeNewline();
    t.writeNewline();

    t.writeSymbols("CPU", "{");
    t.writeNewline();
    t.pushIndent();
    {
        var(t, "Architecture", System::cpuArchitecture());
        var(t, "Num HW Threads", (int)std::thread::hardware_concurrency());
    }
    t.popIndent();
    t.writeSymbols("}");
    t.writeNewline();
    t.writeNewline();
       
    t.writeSymbols("G3D", "{");
    t.writeNewline();
    t.pushIndent();
    {
        const char* g3dPath = getenv("G3D10DATA");
        var(t, "Link version", G3D_VER);
        var(t, "Compile version", System::version());
        var(t, "G3DSpecification::deployMode", _internal::g3dInitializationSpecification().deployMode);
        var(t, "G3D10DATA", String(g3dPath ? g3dPath : ""));
    }
    t.popIndent();
    t.writeSymbols("}");
    t.writeNewline();
    t.writeNewline();
}

String System::currentDateString() {
    time_t t1;
    ::time(&t1);
    tm* t = localtime(&t1);
    return format("%d-%02d-%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday); 
}

String System::currentTimeString() {
    time_t t1;
    ::time(&t1);
    tm* t = localtime(&t1);
    return format("%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
}

}  // namespace G3D
