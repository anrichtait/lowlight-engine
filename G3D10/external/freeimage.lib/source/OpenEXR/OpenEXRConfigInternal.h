// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.

#pragma once

//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

//
// Define and set to 1 if the target system supports a proc filesystem
// compatible with the Linux kernel's proc filesystem.  Note that this
// is only used by a program in the IlmImfTest test suite, it's not
// used by any OpenEXR library or application code.
//

#undef OPENEXR_IMF_HAVE_LINUX_PROCFS

//
// Define and set to 1 if the target system is a Darwin-based system
// (e.g., OS X).
//

/* #undef OPENEXR_IMF_HAVE_DARWIN */

//
// Define and set to 1 if the target system has a complete <iomanip>
// implementation, specifically if it supports the std::right
// formatter.
//

#define OPENEXR_IMF_HAVE_COMPLETE_IOMANIP 1

//
// Define if we can use sysconf(_SC_NPROCESSORS_ONLN) to get CPU count
//

#undef OPENEXR_IMF_HAVE_SYSCONF_NPROCESSORS_ONLN

//
// Define if we can support GCC style inline asm with AVX instructions
//

#undef OPENEXR_IMF_HAVE_GCC_INLINE_ASM_AVX
