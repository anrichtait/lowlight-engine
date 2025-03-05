#include <pybind11/pybind11.h>
#include <iostream>
#include <G3D/G3D.h>
#include <pybind11/operators.h>

namespace py = pybind11;
namespace G3D {

void bind_g3d_string(py::module &m) {
    
    py::class_<G3D::String>(m, "String")
        .def(py::init<>())
        .def(py::init<const char*>())
        .def("__str__", [](const G3D::String &s) { return std::string(s.c_str()); }) // Conversion to Python string
        .def("__repr__", [](const G3D::String &s) { return std::string(s.c_str()); })// Conversion to Python string for representation
        .def("ends_with", [](const G3D::String &g3dString, std::string s){return g3dString.ends_with(s.c_str());})
        .def("add", [](const G3D::String &g3dString, char c){return g3dString + c;}) // Creates copy with c appended
        .def_static("stoi", [](const G3D::String &s){return stoi(s);})
        .def("empty", &G3D::String::empty)
    ;
}

void bind_cframe(py::module &m) {
    
    py::class_<CFrame>(m, "CFrame")
        .def(py::init<>())
        .def(py::init<const Point3&>()) 
        .def(py::init<const Matrix3&, const Point3&>())
        .def(py::init<const Matrix3&>())
        .def_static("fromXYZYPRRadians", &CFrame::fromXYZYPRRadians, "(float x, float y, float z, float yaw = 0.0f, float pitch = 0.0f, float roll = 0.0f);")
        .def_static("fromXYZYPRDegrees", &CFrame::fromXYZYPRDegrees, "(float x, float y, float z, float yaw = 0.0f, float pitch = 0.0f, float roll = 0.0f);")
        .def_readwrite("rotation", &CFrame::rotation)
        .def_readwrite("translation", &CFrame::translation)
    ;
}

template<typename Key, typename Value>
void declare_table(py::module &m, const std::string &keystr, const std::string &valuestr) {

    using Class = Table<Key, Value>;
    // using Iterator = typename Class::Iterator;
    using Entry = typename Class::Entry;

    std::string pyclass_name = std::string("Table_") + keystr + "_" + valuestr;

    py::class_<Entry>(m, (pyclass_name + "_Entry").c_str())
        .def_readonly("key", &Entry::key)
        .def_readonly("value", &Entry::value);
    
    py::class_<Class>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
        .def(py::init<>())
        .def("size", &Class::size)
        .def("get", [](Class &table, const Key& key) {return table.get(key);}, "const Key& key")
        .def("set", &Class::set, "(const Key& key, const Value& value)")
        .def("getKeys", [](Class &table, Array<Key>& arr){return table.getKeys(arr);})
        .def("begin", &Class::begin, py::return_value_policy::reference)
        .def("end", &Class::end, py::return_value_policy::reference)
        .def("__iter__", [](Class& table) { return py::make_iterator(table.begin(), table.end()); },
             py::keep_alive<0, 1>() /* Essential: keep table alive while iterator exists */)
    ;
}

// Binds a table based on its key/value types. 
// Currently only supports minimum necessary to run an app, but adding more should be easy
void bind_table_types(py::module &m){
    
    declare_table<String, Any>(m, "String", "Any");
    // declare_table<String, std::shared_ptr<Model>>(m, "String", "Model"); //should be _SP
    declare_table<String, lazy_ptr<Model>>(m, "String", "Model"); //should be _SP
}

template<typename T>
void declare_array(py::module &m, const std::string &typestr) {
    // In python, the name of the class will be Array_typestr 
    // e.g. Array_int(), Array_Entity_SP()

    using Class = Array<T>;
    std::string pyclass_name = std::string("Array_") + typestr;
    py::class_<Class>(m, pyclass_name.c_str(), py::buffer_protocol(), py::dynamic_attr())
        .def(py::init<>())
        .def(py::init([](const T& val) {return Array<T>(val);}))
        .def("__getitem__", [](Class& array, int index){
            if (index < 0 || index >= array.size()) {
                throw py::index_error("Index out of range");
            }
            return array[index];
        })
        .def("__setitem__", [](Class& array, int index, const T& value) {
            if (index < 0 || index >= array.size()) {
                throw py::index_error("Index out of range");
            }
            array[index] = value;
        })
        .def("append", [](Class& array, const T& value) { array.append(value); })
        .def("size", &Class::size)
        .def("clear", &Class::clear, py::arg("shrink") = true)
        .def("fastRemove", &Class::fastRemove, "(int index, bool shrink = false)")
        .def("resize", &Class::resize, py::arg("n"), py::arg("shrinkIfNecessary") = true)
        .def("push_back", &Class::push_back)
        .def("fastClear", &Class::fastClear)
        // .def("sort", [](Class& array, int dir){array.sort(dir);}, py::arg("direction") = 1) 
    ;
}

// Binds different array types. Currently supports minimum necessary to run app.
void bind_array_types(py::module &m){
    
    declare_array<float>(m, "float");
    declare_array<int>(m, "int");
    declare_array<String>(m, "String");
    declare_array<shared_ptr<Entity>>(m, "Entity_SP");
    declare_array<shared_ptr<Surface>>(m, "Surface_SP");
    declare_array<shared_ptr<Surface2D>>(m, "Surface2D_SP");
    declare_array<Vector3>(m, "Vector3");
    declare_array<shared_ptr<Texture>>(m, "Texture_SP");
    declare_array<shared_ptr<VisibleEntity>>(m, "VisibleEntity_SP"); // i should come up with a better naming scheme for these
    declare_array<Any>(m, "Any");
    declare_array<Box>(m, "Box");
    declare_array<GApp::DebugShape>(m, "DebugShape");
    declare_array<GApp::DebugLabel>(m, "DebugLabel");
    declare_array<const ImageFormat*>(m, "ImageFormat"); //pointer... 
}

void bind_any(py::module &m) {

    py::class_<Any>(m, "Any")
        .def(py::init<>());
    ;
}

void bind_vector_2(py::module &m) {
    
    py::class_<Vector2> vector2_class(m, "Vector2"); 
        vector2_class
        .def(py::init<>())
        .def(py::init<Any>())
        .def(py::init<float, float>()) 
        .def("toAny", [](Vector2& v) {return v.toAny();}, "()")
        .def_readwrite("x", &Vector2::x) 
        .def_readwrite("y", &Vector2::y)
        .def_static("zero", &Vector2::zero)
        ;
    m.attr("Point2") = vector2_class;
}

void bind_vector_3(py::module &m) { // Example of how to alias class name
    
    py::class_<Vector3> vector3_class(m, "Vector3"); 
        vector3_class
        .def(py::init<>())
        .def(py::init<Any>())
        .def(py::init<float, float, float>()) 
        .def("toAny", [](Vector3& v) {return v.toAny();}, "()")
        .def_readwrite("x", &Vector3::x)
        .def_readwrite("y", &Vector3::y)
        .def_readwrite("z", &Vector3::z)
        .def_static("zero", &Vector3::zero)
        .def("magnitude", &Vector3::magnitude)
        ;
    m.attr("Point3") = vector3_class;
}

void bind_color_3(py::module &m) {

    py::class_<Color3>(m, "Color3")
        .def(py::init<>())
        .def(py::init<float>())
        .def(py::init<float, float, float>())
        .def("__mul__", [](const Color3& c1, float s) { return c1 * s; })
        .def("__mul__", [](float s, const Color3& c1) { return c1 * s; })
        .def_static("red", &Color3::red)
        .def_static("green", &Color3::green)
        .def_static("blue", &Color3::blue)
        .def_static("purple", &Color3::purple)
        .def_static("cyan", &Color3::cyan)
        .def_static("yellow", &Color3::yellow)
        .def_static("brown", &Color3::brown)
        .def_static("orange", &Color3::orange)
        .def_static("black", &Color3::black)
        .def_static("gray", &Color3::gray)
        .def_static("white", &Color3::white)
        .def_static("zero", &Color3::zero)
        .def_static("one", &Color3::one)
        .def_static("nan", &Color3::nan);   
    ;
}

void bind_color_4(py::module &m) {

    py::class_<Color4>(m, "Color4")
        .def(py::init<const Color3&, float>(), py::arg("color"), py::arg("alpha") = 1.0f)
    ;
}

void bind_fovdirection(py::module &m) {

    py::enum_<FOVDirection::Value>(m, "FOVDirection")
        .value("HORIZONTAL", FOVDirection::HORIZONTAL)
        .value("VERTICAL", FOVDirection::VERTICAL)
        .value("DIAGONAL", FOVDirection::DIAGONAL)
    ;
}

void bind_spline_extrapolation_mode(py::module &m) {

    py::enum_<SplineExtrapolationMode::Value>(m, "SplineExtrapolationMode")
        .value("CYCLIC", SplineExtrapolationMode::CYCLIC)
        .value("LINEAR", SplineExtrapolationMode::LINEAR)
        .value("CLAMP", SplineExtrapolationMode::CLAMP)
    ;

    py::enum_<SplineInterpolationMode::Value>(m, "SplineInterpolationMode")
        .value("LINEAR", SplineInterpolationMode::LINEAR)
        .value("CUBIC", SplineInterpolationMode::CUBIC)
    ;
}

// Writes to console, but python can just use print
void bind_text_output(py::module &m) {

    py::class_<TextOutput>(m, "TextOutput")
        .def(py::init<const TextOutput::Settings&>())
        .def(py::init<const String&, const TextOutput::Settings&>())
        .def("writeSymbol", py::overload_cast<const String&>(&TextOutput::writeSymbol))
        .def("writeSymbol", py::overload_cast<char>(&TextOutput::writeSymbol))
        .def("writeSymbols", &TextOutput::writeSymbols,
          py::arg("a"),
          py::arg("b") = String(""),  // Default argument value
          py::arg("c") = String(""),  // Default argument value
          py::arg("d") = String(""),  // Default argument value
          py::arg("e") = String(""),  // Default argument value
          py::arg("f") = String(""))  // Default argument value
        .def("pushIndent", &TextOutput::pushIndent)
        .def("popIndent", &TextOutput::popIndent)
        .def("writeNewline", &TextOutput::writeNewline)
        .def("commitString", py::overload_cast<String&>(&TextOutput::commitString))
    ;

    py::class_<TextOutput::Settings>(m, "TextOutputSettings")
        .def(py::init<>())
    ;
}

void bind_system(py::module &m) {

    py::class_<System>(m, "System")
        .def_static("describeSystem", py::overload_cast<class TextOutput&>(&System::describeSystem))
        .def_static("findDataFile", &System::findDataFile, py::arg("str"), py::arg("exceptionIfNotFound") = true, py::arg("caseSensitive") = true) // case sensitivity default is False on Windows, True otherwise
        .def_static("time", &System::time, "()")
        .def_static("currentProgramFilename", &System::currentProgramFilename, "()")
        .def_static("setAppDataDir", &System::setAppDataDir, "(datadir)")
        .def_static("setAppDataDirs", &System::setAppDataDirs, "(datadirs)")
        .def_static("g3dRevision", &System::g3dRevision)
        .def_static("cleanup", &System::cleanup)
    ;
}

void bind_network_device(py::module &m) {

    py::class_<NetworkDevice>(m, "NetworkDevice")
        .def_static("instance", &NetworkDevice::instance, "()")
        .def("describeSystem", py::overload_cast<TextOutput&>(&NetworkDevice::describeSystem))
        .def_static("cleanup", &NetworkDevice::cleanup)
    ;
}

void bind_box(py::module &m) {

    py::class_<Box>(m, "Box")
        .def(py::init<Vector3, Vector3>())
    ;
}

void bind_file_system(py::module &m) {

    py::class_<FileSystem>(m, "FileSystem")
        .def_static("exists", &FileSystem::exists)
        .def_static("resolve", &FileSystem::resolve, py::arg("path"), py::arg("cwd") = FileSystem::currentDirectory())
        .def_static("isDirectory", &FileSystem::isDirectory, py::arg("path"), py::arg("expandEnvironmentVariablesInPath") = true)
        .def_static("getDirectories", &FileSystem::getDirectories, py::arg("spec"), py::arg("result"), py::arg("includeParentPath") = false)
        .def_static("getFiles", &FileSystem::getFiles, py::arg("spec"), py::arg("result"), py::arg("includeParentPath") = false)
        .def_static("currentDirectory", &FileSystem::currentDirectory)
    ;

    py::class_<FilePath>(m, "FilePath")
        .def_static("concat", &FilePath::concat)
        .def_static("makeLegalFilename", &FilePath::makeLegalFilename, py::arg("f"), py::arg("maxLength") = 100000)
    ;
}

void bind_rect_2d(py::module &m) {

    py::class_<Rect2D>(m, "Rect2D")
        .def_static("xywh", py::overload_cast<float, float, float, float>(&Rect2D::xywh))
    ;
}

void bind_vector_2_int_32(py::module &m) {

    py::class_<Vector2int32>(m, "Vector2int32")
        .def(py::init<int, int>())
        .def(py::init<Vector2>())
        .def_readwrite("x", &Vector2int32::x)
        .def_readwrite("y", &Vector2int32::y)
    ;
}

void bind_vector_2_int_16(py::module &m) {

    py::class_<Vector2int16>(m, "Vector2int16")
        .def(py::init<int, int>())
        .def("__sub__", [](Vector2int16& v1, Vector2int16& v2){return v1 - v2;})
        .def_readwrite("x", &Vector2int16::x)
        .def_readwrite("y", &Vector2int16::y)
    ;
}

void bind_image_format(py::module &m) {

    py::class_<ImageFormat>(m, "ImageFormat")
        .def_readonly("stencilBits", &ImageFormat::stencilBits)
        .def_static("AUTO", &ImageFormat::AUTO)
        .def_static("L8", &ImageFormat::L8)
        .def_static("L16", &ImageFormat::L16)
        .def_static("L16F", &ImageFormat::L16F)
        .def_static("L32F", &ImageFormat::L32F)
        .def_static("A8", &ImageFormat::A8)
        .def_static("A16", &ImageFormat::A16)
        .def_static("A16F", &ImageFormat::A16F)
        .def_static("A32F", &ImageFormat::A32F)
        .def_static("LA4", &ImageFormat::LA4)
        .def_static("LA8", &ImageFormat::LA8)
        .def_static("LA16", &ImageFormat::LA16)
        .def_static("LA16F", &ImageFormat::LA16F)
        .def_static("LA32F", &ImageFormat::LA32F)
        .def_static("RGB5", &ImageFormat::RGB5)
        .def_static("RGB5A1", &ImageFormat::RGB5A1)
        .def_static("RGB8", &ImageFormat::RGB8)
        .def_static("RGB10", &ImageFormat::RGB10)
        .def_static("RGB10A2", &ImageFormat::RGB10A2)
        .def_static("RGB16", &ImageFormat::RGB16)
        .def_static("RGB16F", &ImageFormat::RGB16F)
        .def_static("RGB32F", &ImageFormat::RGB32F)
        .def_static("R11G11B10F", &ImageFormat::R11G11B10F)
        .def_static("RGB9E5F", &ImageFormat::RGB9E5F)
        .def_static("RGB8I", &ImageFormat::RGB8I)
        .def_static("RGB8UI", &ImageFormat::RGB8UI)
        .def_static("RGBA8I", &ImageFormat::RGBA8I)
        .def_static("RGBA8UI", &ImageFormat::RGBA8UI)
        .def_static("RGB8_SNORM", &ImageFormat::RGB8_SNORM)
        .def_static("RGBA8_SNORM", &ImageFormat::RGBA8_SNORM)
        .def_static("RGB16_SNORM", &ImageFormat::RGB16_SNORM)
        .def_static("RGBA16_SNORM", &ImageFormat::RGBA16_SNORM)
        .def_static("BGR8", &ImageFormat::BGR8)
        .def_static("BGRA8", &ImageFormat::BGRA8)
        .def_static("R8", &ImageFormat::R8)
        .def_static("R8I", &ImageFormat::R8I)
        .def_static("R8UI", &ImageFormat::R8UI)
        .def_static("R8_SNORM", &ImageFormat::R8_SNORM)
        .def_static("R16", &ImageFormat::R16)
        .def_static("R16I", &ImageFormat::R16I)
        .def_static("R16UI", &ImageFormat::R16UI)
        .def_static("R16_SNORM", &ImageFormat::R16_SNORM)
        .def_static("R16F", &ImageFormat::R16F)
        .def_static("R32I", &ImageFormat::R32I)
        .def_static("R32UI", &ImageFormat::R32UI)
        .def_static("RG8", &ImageFormat::RG8)
        .def_static("RG8I", &ImageFormat::RG8I)
        .def_static("RG8UI", &ImageFormat::RG8UI)
        .def_static("RG8_SNORM", &ImageFormat::RG8_SNORM)
        .def_static("RG16", &ImageFormat::RG16)
        .def_static("RG16I", &ImageFormat::RG16I)
        .def_static("RG16UI", &ImageFormat::RG16UI)
        .def_static("RG16_SNORM", &ImageFormat::RG16_SNORM)
        .def_static("RG16F", &ImageFormat::RG16F)
        .def_static("RG32I", &ImageFormat::RG32I)
        .def_static("RG32UI", &ImageFormat::RG32UI)
        .def_static("R32F", &ImageFormat::R32F)
        .def_static("RG32F", &ImageFormat::RG32F)
        .def_static("RGBA8", &ImageFormat::RGBA8)
        .def_static("RGBA16", &ImageFormat::RGBA16)
        .def_static("RGBA16F", &ImageFormat::RGBA16F)
        .def_static("RGBA32F", &ImageFormat::RGBA32F)
        .def_static("RGBA16I", &ImageFormat::RGBA16I)
        .def_static("RGBA16UI", &ImageFormat::RGBA16UI)
        .def_static("RGB32I", &ImageFormat::RGB32I)
        .def_static("RGB32UI", &ImageFormat::RGB32UI)
        .def_static("RGBA32I", &ImageFormat::RGBA32I)
        .def_static("RGBA32UI", &ImageFormat::RGBA32UI)
        .def_static("RGBA4", &ImageFormat::RGBA4)
        .def_static("RGBA2", &ImageFormat::RGBA2)
        .def_static("YUV420_PLANAR", &ImageFormat::YUV420_PLANAR)
        .def_static("YUV422", &ImageFormat::YUV422)
        .def_static("YUV444", &ImageFormat::YUV444)
        .def_static("RGB_DXT1", &ImageFormat::RGB_DXT1)
        .def_static("RGBA_DXT1", &ImageFormat::RGBA_DXT1)
        .def_static("RGBA_DXT3", &ImageFormat::RGBA_DXT3)
        .def_static("RGBA_DXT5", &ImageFormat::RGBA_DXT5)
        .def_static("SRGB8", &ImageFormat::SRGB8)
        .def_static("SRGBA8", &ImageFormat::SRGBA8)
        .def_static("SL8", &ImageFormat::SL8)
        .def_static("SLA8", &ImageFormat::SLA8)
        .def_static("SRGB_DXT1", &ImageFormat::SRGB_DXT1)
        .def_static("SRGBA_DXT1", &ImageFormat::SRGBA_DXT1)
        .def_static("SRGBA_DXT3", &ImageFormat::SRGBA_DXT3)
        .def_static("SRGBA_DXT5", &ImageFormat::SRGBA_DXT5)
        .def_static("DEPTH16", &ImageFormat::DEPTH16)
        .def_static("DEPTH24", &ImageFormat::DEPTH24)
        .def_static("DEPTH32", &ImageFormat::DEPTH32)
        .def_static("DEPTH32F", &ImageFormat::DEPTH32F)
        .def_static("STENCIL1", &ImageFormat::STENCIL1)
        .def_static("STENCIL4", &ImageFormat::STENCIL4)
        .def_static("STENCIL8", &ImageFormat::STENCIL8)
        .def_static("STENCIL16", &ImageFormat::STENCIL16)
        .def_static("DEPTH24_STENCIL8", &ImageFormat::DEPTH24_STENCIL8)
    ;
}

void bind_frame_name(py::module &m) {

    py::enum_<FrameName::Value>(m, "FrameName")
        .value("NONE", FrameName::NONE)
        .value("WORLD", FrameName::WORLD)
        .value("OBJECT", FrameName::OBJECT)
        .value("CAMERA", FrameName::CAMERA)
        .value("LIGHT", FrameName::LIGHT)
        .value("TANGENT", FrameName::TANGENT)
        .value("TEXTURE", FrameName::TEXTURE)
        .value("SCREEN", FrameName::SCREEN)
    ;
}

void bind_aabox(py::module &m) {

    py::class_<AABox>(m, "AABox")
        .def(py::init<>())
        .def("extent", [](AABox a){return a.extent();})
        .def("center", &AABox::center)
    ;
}

void bind_image(py::module &m) {

    py::class_<Image, std::shared_ptr<Image>>(m, "Image")
        .def("save", &Image::save)
        .def("save", [](std::shared_ptr<Image> img, std::string path){img->save(G3D::String(path));})
    ;
}

void bind_g3d_base(py::module &m){

    bind_g3d_string(m);
    bind_cframe(m);
    bind_table_types(m);
    bind_array_types(m);
    bind_any(m);
    bind_vector_2(m);
    bind_vector_3(m);
    bind_color_3(m);
    bind_color_4(m);
    bind_fovdirection(m);
    bind_spline_extrapolation_mode(m);
    bind_text_output(m);
    bind_system(m);
    bind_network_device(m);
    bind_box(m);
    bind_file_system(m);
    bind_rect_2d(m);
    bind_vector_2_int_32(m);
    bind_vector_2_int_16(m);
    bind_image_format(m);
    bind_frame_name(m);
    bind_aabox(m); 
    bind_image(m);

    m.def("toLower", &toLower);
    m.def("generateFileNameBaseAnySuffix", &generateFileNameBaseAnySuffix);
}
}
