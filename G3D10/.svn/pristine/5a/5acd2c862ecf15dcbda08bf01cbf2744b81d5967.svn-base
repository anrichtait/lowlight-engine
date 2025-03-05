#include <pybind11/pybind11.h>
#include <pybind11/stl_bind.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <iostream>
#include <G3D/G3D.h>
#include "pyG3D.h"
#include "g3d_app_bindings.h"
#include "g3d_base_bindings.h"
#include "g3d_gfx_bindings.h"

/**
 * Notes about bindings (for those interested in adding more):
 * 
 * - If multiple implementations of the same function name exist, need to explicitly specify the arguments using py::overload_cast
 * 
 * - Constructor declarations don't need py::overload_cast because the args are already specified
 * 
 * - The class bound for an enum must be the lowest level name, which sometimes is not the one reflected in enum's usage in c++ code
 * 
 * - For default arguments, add arguments py::arg("argname") = default value
 * 
 * - The types of default arguments must be bound before the function default argument is bound, or else pybind will claim that the type is not bound.
 * 
 * - If the default argument is a g3d string, it must be explicitly initialized as such (it won't just cast to g3d string automatically)
 * 
 * - Can define lambda functions as [](args){body;}
 * 
 * - For non-static lambda functions, letting the first arg be the object will allow you to call the function with obj.func(rest of the args)
 * 
 * - For the previous bullet point, make sure that the first arg type is a reference instead of a copy :') (i.e. Classname& classObject)
 * 
*/


namespace py = pybind11;
namespace G3D {

PYG3D_MODULE_DECLARE {

    m.doc() = "G3D but in python!";

    py::bind_vector<std::vector<G3D::String>>(m, "StringVector"); //used for gapp::settings::datadirs

    bind_g3d_base(m);
    bind_g3d_gfx(m);
    bind_g3d_app(m);

    m.def("getStencilBits", [](GApp::Settings& s){return GLCaps::firstSupportedTexture(s.hdrFramebuffer.preferredDepthFormats)->stencilBits;});
    m.def("checkImageNone", [](GApp::Settings& s){return GLCaps::firstSupportedTexture(s.hdrFramebuffer.preferredDepthFormats) == nullptr;});
}
} // namespace G3D
