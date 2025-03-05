/** pyG3D.h 
 * 
 * Note: the G3D.h include must go AFTER the pybind11.h include since python 3.10
 * has a struct called CFrame and this collides with the G3D CFrame typedef since
 * G3D.h has a global using namespace G3D;
 */

#include <pybind11/pybind11.h>
#include <G3D/G3D.h>
#include <iostream>

#ifndef G3D_PYG3D_H
#define G3D_PYG3D_H

// Macro to generate module name based on Python version e.g. pyG3D_3_11
#ifdef _DEBUG
#   define PYG3D_NAME_WITH_PY_VERSION(major_version, minor_version) pyG3D##_##major_version##_##minor_version##d
#else
#   define PYG3D_NAME_WITH_PY_VERSION(major_version, minor_version) pyG3D##_##major_version##_##minor_version
#endif

#define EXPAND_PY_VERSIONS(maj_ver, min_ver) PYG3D_NAME_WITH_PY_VERSION(maj_ver, min_ver)

#define PYG3D_MODULE_NAME EXPAND_PY_VERSIONS(PY_MAJOR_VERSION, PY_MINOR_VERSION)

// Wrapper to expand macro module name before calling PYBIND11_MODULE
#define PYBIND11_MODULE_DECLARE(name) PYBIND11_MODULE(name, m)

#define PYG3D_MODULE_DECLARE PYBIND11_MODULE_DECLARE(PYG3D_MODULE_NAME)

#endif // G3D_PYG3D_H
