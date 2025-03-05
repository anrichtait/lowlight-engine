#ifndef ASSIMP_REVISION_H_INC
#define ASSIMP_REVISION_H_INC

/** G3D Change
 * Made GitVersion 0 and GitBranch an empty string
 */
#define GitVersion 0x0
#define GitBranch ""
/* END G3D CHANGE */

/** G3D Change
 * Hard coded the version numbers and comment out any CMake substitutions
 */
#define VER_MAJOR 5 // @ASSIMP_VERSION_MAJOR@
#define VER_MINOR 3 // @ASSIMP_VERSION_MINOR@
#define VER_PATCH 1 // @ASSIMP_VERSION_PATCH@
// #define VER_BUILD @ASSIMP_PACKAGE_VERSION@ 
/* END G3D CHANGE */

#define STR_HELP(x) #x
#define STR(x) STR_HELP(x)

#define VER_FILEVERSION             VER_MAJOR,VER_MINOR,VER_PATCH,VER_BUILD
#if (GitVersion == 0)
#define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD)
#else
#define VER_FILEVERSION_STR         STR(VER_MAJOR) "." STR(VER_MINOR) "." STR(VER_PATCH) "." STR(VER_BUILD) " (Commit @GIT_COMMIT_HASH@)"
#endif
#define VER_COPYRIGHT_STR           "\xA9 2006-2023"

#ifdef  NDEBUG
#define VER_ORIGINAL_FILENAME_STR   "@CMAKE_SHARED_LIBRARY_PREFIX@assimp@LIBRARY_SUFFIX@.dll"
#else
#define VER_ORIGINAL_FILENAME_STR   "@CMAKE_SHARED_LIBRARY_PREFIX@assimp@LIBRARY_SUFFIX@@CMAKE_DEBUG_POSTFIX@.dll"
#endif //  NDEBUG

#endif // ASSIMP_REVISION_H_INC
