// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.

// This file is auto-generated by the configure step

#ifndef INCLUDED_OPENEXR_CONFIG_H
#define INCLUDED_OPENEXR_CONFIG_H 1

#pragma once

//
// Current internal library namepace name
//
#define OPENEXR_IMF_INTERNAL_NAMESPACE_CUSTOM 1
#define OPENEXR_IMF_INTERNAL_NAMESPACE Imf_2_5

//
// Current public user namepace name
//

/* #undef OPENEXR_IMF_NAMESPACE_CUSTOM */
#define OPENEXR_IMF_NAMESPACE Imf

//
// Version string for runtime access
//

#define OPENEXR_VERSION_STRING "2.5.9"
#define OPENEXR_PACKAGE_STRING "openEXR 2.5.9"

#define OPENEXR_VERSION_MAJOR 2
#define OPENEXR_VERSION_MINOR 5
#define OPENEXR_VERSION_PATCH 9

// Version as a single hex number, e.g. 0x01000300 == 1.0.3
#define OPENEXR_VERSION_HEX ((uint32_t(OPENEXR_VERSION_MAJOR) << 24) | \
                             (uint32_t(OPENEXR_VERSION_MINOR) << 16) | \
                             (uint32_t(OPENEXR_VERSION_PATCH) <<  8))

#endif // INCLUDED_OPENEXR_CONFIG_H
