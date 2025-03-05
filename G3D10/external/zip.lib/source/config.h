#ifndef HAD_CONFIG_H
#define HAD_CONFIG_H
#ifndef _HAD_ZIPCONF_H
#include "zipconf.h"
#endif
/* BEGIN DEFINES */
/* #undef HAVE___PROGNAME */
#define HAVE__CHMOD
#define HAVE__CLOSE
#define HAVE__DUP
#define HAVE__FDOPEN
#define HAVE__FILENO
#define HAVE__OPEN
#define HAVE__SETMODE
/* #undef HAVE__SNPRINTF */
#define HAVE__STRDUP
#define HAVE__STRICMP
#define HAVE__STRTOI64
#define HAVE__STRTOUI64
#define HAVE__UMASK
#define HAVE__UNLINK
/* #undef HAVE_CLONEFILE */
#define HAVE_FILENO
#ifndef _WIN32
#define HAVE_FSEEKO
#define HAVE_FTELLO
#endif
/* #undef HAVE_GETPROGNAME */
/* #undef HAVE_LIBBZ2 */
#define HAVE_OPEN
/* #undef HAVE_MKSTEMP */
#define HAVE_SETMODE
/* #undef HAVE_SNPRINTF */
#ifndef _WIN32
#define HAVE_SSIZE_T_LIBZIP
#define HAVE_STRCASECMP
#endif
#define HAVE_STRDUP
#define HAVE_STRICMP
#define HAVE_STRTOLL
#define HAVE_STRTOULL
/* #undef HAVE_STRUCT_TM_TM_ZONE */
#define HAVE_STDBOOL_H
/* #undef HAVE_STRINGS_H */

#ifndef _WIN32
#define HAVE_UNISTD_H
#endif

#define __INT8_LIBZIP 1
#define INT8_T_LIBZIP 1
#define UINT8_T_LIBZIP 1
#define __INT16_LIBZIP 2
#define INT16_T_LIBZIP 2
#define UINT16_T_LIBZIP 2
#define __INT32_LIBZIP 4
#define INT32_T_LIBZIP 4
#define UINT32_T_LIBZIP 4
#define __INT64_LIBZIP 8
#define INT64_T_LIBZIP 8
#define UINT64_T_LIBZIP 8
#define SIZEOF_OFF_T 4
#define SIZE_T_LIBZIP 4
/* #undef SSIZE_T_LIBZIP */
/* #undef HAVE_DIRENT_H */
/* #undef HAVE_FTS_H */
/* #undef HAVE_NDIR_H */
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_SYS_NDIR_H */
/* #undef WORDS_BIGENDIAN */
/* #undef HAVE_SHARED */
/* END DEFINES */
#define PACKAGE "libzip"
#define VERSION "1.4.0"

#ifndef HAVE_SSIZE_T_LIBZIP
#  if SIZE_T_LIBZIP == INT_LIBZIP
#ifndef __APPLE__
typedef int ssize_t;
#endif
#  elif SIZE_T_LIBZIP == LONG_LIBZIP
typedef long ssize_t;
#  elif SIZE_T_LIBZIP == LONG_LONG_LIBZIP
typedef long long ssize_t;
#  else
#error no suitable type for ssize_t found
#  endif
#endif

#endif /* HAD_CONFIG_H */
