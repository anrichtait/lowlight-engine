/**
  \file G3D-base.lib/include/G3D-base/platform.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define G3D_platform_h

/**
 \def G3D_VER

 The version number of G3D in the form: MmmBB ->
 version M.mm [beta BB]
 */
#define G3D_VER 100100

#if defined(G3D_RELEASEDEBUG)
#   define G3D_DEBUGRELEASE
#endif

#if defined(G3D_DEBUGRELEASE) && defined(_DEBUG)
#   undef _DEBUG
#endif

/** \def G3D_DEBUG
    Defined if G3D is built in debug mode. */
#if !defined(G3D_DEBUG) && (defined(_DEBUG) || defined(G3D_DEBUGRELEASE))
#   define G3D_DEBUG
#endif

/** \def G3D_ARM 
   Defined on ARM processors */

/** \def G3D_X86
   Defined on x86 processors */
#if defined(__arm64__) || defined(__ARM_ARCH) || defined(__arm) || defined (__aarch64__)
#    define G3D_ARM
#elif defined(__x86_64__) || defined(__IA64__) || defined(_M_X64) || defined(_M_IA64)
#    define G3D_X86
#endif

#if defined(__arm64__) || defined(__x86_64__) || defined(__IA64__) || defined(_M_X64) || defined(_M_IA64) || defined (__aarch64__)
#    define G3D_64_BIT
#endif


/** \def G3D_WINDOWS Defined on Windows platforms*/
/** \def G3D_FREEBSD Defined on FreeBSD and OpenBSD */
/** \def G3D_LINUX Defined on Linux, FreeBSD and OpenBSD */
/** \def G3D_OSX Defined on MacOS */

#ifdef _MSC_VER
#   define G3D_WINDOWS
#elif defined(__APPLE__)
#   define G3D_OSX

    // Prevent OS X fp.h header from being included; it defines
    // pi as a constant, which creates a conflict with G3D
#   define __FP__
#elif  defined(__FreeBSD__) || defined(__OpenBSD__)
    #define G3D_FREEBSD
    #define G3D_LINUX
#elif defined(__linux__)
    #define G3D_LINUX
#else
    #error Unknown platform
#endif

// Verify that the supported compilers are being used
#if defined(G3D_LINUX) || defined(G3D_OSX)
#   ifndef __clang__
#       error G3D requires using clang to compile.
#   endif
#endif


#ifdef _MSC_VER
// Turn off warnings about deprecated C routines
#   pragma warning (disable : 4996)

// Turn off "conditional expression is constant" warning; MSVC generates this
// for debug assertions in inlined methods.
#  pragma warning (disable : 4127)

// Prevent Winsock conflicts by hiding the winsock API
#   ifndef _WINSOCKAPI_
#       define _G3D_INTERNAL_HIDE_WINSOCK_
#       define _WINSOCKAPI_
#   endif

// Disable 'name too long for browse information' warning
#   pragma warning (disable : 4786)

#   define restrict

/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_PRINTF_ARGS

/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_VPRINTF_ARGS

/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_PRINTF_METHOD_ARGS

/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_VPRINTF_METHOD_ARGS

    // On MSVC, we need to link against the multithreaded DLL version of
    // the C++ runtime because that is what SDL and ZLIB are compiled
    // against.  This is not the default for MSVC, so we set the following
    // defines to force correct linking.
    //
    // For documentation on compiler options, see:
    //  http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/_core_.2f.md.2c_2f.ml.2c_2f.mt.2c_2f.ld.asp
    //  http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore98/HTML/_core_Compiler_Reference.asp
    //

    // DLL runtime
    #ifndef _DLL
        #define _DLL
    #endif

    // Multithreaded runtime
    #ifndef _MT
        #define _MT 1
    #endif

    // Ensure that we aren't forced into the static lib
    #ifdef _STATIC_CPPLIB
        #undef _STATIC_CPPLIB
    #endif

#ifdef _DEBUG
    // Some of the support libraries are always built in Release.
    // Make sure the debug runtime library is linked in
    #pragma comment(linker, "/NODEFAULTLIB:MSVCRT.LIB")
    #pragma comment(linker, "/NODEFAULTLIB:MSVCPRT.LIB")
#endif


#    ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN 1
#    endif

#   ifndef NOMINMAX
#       define NOMINMAX 1
#   endif
#   ifndef _WIN32_WINNT
#       define _WIN32_WINNT 0x0602
#   endif
#   include <windows.h>
#   undef WIN32_LEAN_AND_MEAN
#   undef NOMINMAX

#   ifdef _G3D_INTERNAL_HIDE_WINSOCK_
#      undef _G3D_INTERNAL_HIDE_WINSOCK_
#      undef _WINSOCKAPI_
#   endif


/** \def G3D_START_AT_MAIN()
    Makes Windows programs using the WINDOWS subsystem invoke main() at program start by
    defining a WinMain(). Does nothing on other operating systems.*/
#   define G3D_START_AT_MAIN()\
int WINAPI G3D_WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw);\
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR szCmdLine, int sw) {\
    return G3D_WinMain(hInst, hPrev, szCmdLine, sw);\
}

#else

/** @def G3D_START_AT_MAIN()
    Defines necessary wrapper around WinMain on Windows to allow transfer of execution to main(). */
#   define G3D_START_AT_MAIN()

#endif  // win32

#ifdef __GNUC__

#    include <stdint.h>

#   if __STDC_VERSION__ < 199901
#        define restrict __restrict__
#   endif


/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_PRINTF_METHOD_ARGS   __attribute__((__format__(__printf__, 2, 3)))

/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_VPRINTF_METHOD_ARGS  __attribute__((__format__(__printf__, 2, 0)))

/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_PRINTF_ARGS          __attribute__((__format__(__printf__, 1, 2)))

/** @def G3D_CHECK_PRINTF_METHOD_ARGS()
    Enables printf parameter validation on gcc. */
#   define G3D_CHECK_VPRINTF_ARGS         __attribute__((__format__(__printf__, 1, 0)))
#endif


/**
  \def STR(expression)

  Creates a string from the expression.  Frequently used with G3D::Shader
  to express shading programs inline.

  <CODE>STR(this becomes a string)\verbatim<PRE>\endverbatim evaluates the same as \verbatim<CODE>\endverbatim"this becomes a string"</CODE>
 */
#define STR(x) #x

/** @def PRAGMA(expression)
    \#pragma may not appear inside a macro, so this uses the pragma operator
    to create an equivalent statement.*/
#ifdef _MSC_VER
// Microsoft's version http://msdn.microsoft.com/en-us/library/d9x1s805.aspx
#    define PRAGMA(x) __pragma(x)
#else
// C99 standard http://www.delorie.com/gnu/docs/gcc/cpp_45.html
#    define PRAGMA(x) _Pragma(#x)
#endif

/** \def G3D_BEGIN_PACKED_CLASS(byteAlign)
    Switch to tight alignment.

    \code
    G3D_BEGIN_PACKED_CLASS(1)
    ThreeBytes {
    public:
        uint8 a, b, c;
    }
    G3D_END_PACKED_CLASS(1)
    \endcode


    See G3D::Color3uint8 for an example.*/
#ifdef __GNUC__
#    define G3D_BEGIN_PACKED_CLASS(byteAlign)  class __attribute((__packed__))
#elif defined(_MSC_VER)
#    define G3D_BEGIN_PACKED_CLASS(byteAlign)  PRAGMA( pack(push, byteAlign) ) class
#else
#    define G3D_BEGIN_PACKED_CLASS(byteAlign)  class
#endif

/** \def G3D_END_PACKED_CLASS(byteAlign)
    End switch to tight alignment

    See G3D::Color3uint8 for an example.*/
#ifdef __GNUC__
#    define G3D_END_PACKED_CLASS(byteAlign)  __attribute((aligned(byteAlign))) ;
#elif defined(_MSC_VER)
#    define G3D_END_PACKED_CLASS(byteAlign)  ; PRAGMA( pack(pop) )
#else
#    define G3D_END_PACKED_CLASS(byteAlign)  ;
#endif

// Defining this supresses a warning in Visual Studio 2017 15.8+
// where aligned_storage (used by shared_ptr) enables standards
// conforming behavior of using the alignment of the ref counted objects
// instead of being clamped to max_align_t as before.
#define _ENABLE_EXTENDED_ALIGNED_STORAGE

// Bring in shared_ptr and weak_ptr
#include <memory>
using std::shared_ptr;
using std::weak_ptr;
using std::dynamic_pointer_cast;
using std::static_pointer_cast;
using std::enable_shared_from_this;


namespace G3D {
    /** Options for initG3D and initGLG3D. */
    class G3DSpecification {
    public:
        /**
          \brief Should G3D spawn its own network thread?

          If true, G3D will spawn a thread for network management on the first invocation of G3D::NetServer::create or
          G3D::NetConnection::connectToServer.

          If false and networking is used, the application must explicitly invoke G3D::serviceNetwork() regularly to allow the network
          code to run.

          In either case, the network API is threadsafe.

          Default: true.
        */
        bool threadedNetworking = true;

        /** Should AudioDevice be enabled? (It will still be initialized regardless of enabling.) Default: false. */
        bool audio = false;

        /** Audio DSP buffer length to use. Affects audio latency. Default: 1024. 
        FMOD claims that the default results in 21.33 ms of latency at 48 khz (1024 / 48000 * 1000 = 21.33). */
        unsigned int audioBufferLength = 1024;

        /** Number of audio DSP buffers to use. Default: 4. */
        int audioNumBuffers = 4;

        /** Set parameters for deployment of a standalone application.
            If true, disable System::findDataFile's ability to look in
            the directory specified by the G3D10DATA environment variable. */
        bool deployMode = false;

        /** Name that Log::common() and logPrintf() use */
        const char* logFilename = "log.txt";

        /** Scale used by G3D::GuiWindow::pixelScale.
            If -1, the scale automatically is chosen by the GuiWindow based on the 
            primary display resolution. 4k = 2x, 8k = 4x */
        float defaultGuiPixelScale = -1.0f;

        G3DSpecification() {}

        virtual ~G3DSpecification() {}
    };


    namespace _internal {
        /** Set by initG3D, defined in initG3D.cpp */
        G3DSpecification& g3dInitializationSpecification();
    }
}

// See http://stackoverflow.com/questions/2670816/how-can-i-use-the-compile-time-constant-line-in-a-string
// For use primarily with NUMBER_TO_STRING(__LINE__)
#define NUMBER_TO_STRING(x) NUMBER_TO_STRING2(x)
#define NUMBER_TO_STRING2(x) #x
#define __LINE_AS_STRING__ NUMBER_TO_STRING(__LINE__)

#ifdef nil
#   undef nil
#endif

#if ! defined(G3D_WINDOWS) && ! defined(G3D_OSX)
   // FMOD is not supported by G3D on other platforms...yet
#    define G3D_NO_FMOD
#endif

/** \def G3D_MIN_OPENGL_VERSION
    Minimum version of OpenGL required by G3D on this platform, mulitplied by 100 to create an integer, e.g., 330 = 3.30.
    Assumes OpenGL core if > 300
*/
#define G3D_MIN_OPENGL_VERSION 410

#define TBB_IMPLEMENT_CPP0X 0
#define __TBB_NO_IMPLICIT_LINKAGE 1
#define __TBBMALLOC_NO_IMPLICIT_LINKAGE 1
#include <tbb/tbb.h>

