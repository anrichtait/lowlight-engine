#include <pybind11/pybind11.h>
#include <iostream>
#include <G3D/G3D.h>

namespace py = pybind11;
namespace G3D {

void bind_render_device(py::module &m) {
   
    py::class_<RenderDevice>(m, "RenderDevice")
        .def(py::init<>())
        .def_static("new", [](){ return new RenderDevice();})
        .def("init", py::overload_cast<OSWindow*>(&RenderDevice::init))
        .def("init", py::overload_cast<const OSWindow::Settings&>(&RenderDevice::init))
        .def("pushState", py::overload_cast<>(&RenderDevice::pushState), "()") 
        .def("pushState", py::overload_cast<const shared_ptr<Framebuffer>&>(&RenderDevice::pushState))
        .def("popState", &RenderDevice::popState, "()")
        .def("push2D", py::overload_cast<>(&RenderDevice::push2D), "()")
        .def("pop2D", &RenderDevice::pop2D, "()")
        .def("setProjectionAndCameraMatrix", &RenderDevice::setProjectionAndCameraMatrix, "(const Projection &P, const CFrame &C)")
        .def("clear", py::overload_cast<>(&RenderDevice::clear), "()")
        .def("setColorClearValue", &RenderDevice::setColorClearValue, "(color4)")
        .def("setColorClearValue", [](RenderDevice& rd, const Color3& color){rd.setColorClearValue(color);})
        .def("window", &RenderDevice::window, "()")
        .def("describeSystem", py::overload_cast<TextOutput&>(&RenderDevice::describeSystem))
        .def("width", &RenderDevice::width, "()")
        .def("setSwapBuffersAutomatically", &RenderDevice::setSwapBuffersAutomatically, "(bool)")
        .def("beginFrame", &RenderDevice::beginFrame)
        .def("endFrame", &RenderDevice::endFrame)
        .def("swapBuffersAutomatically", &RenderDevice::swapBuffersAutomatically)
        .def("swapBuffers", &RenderDevice::swapBuffers)
        .def("screenshotPic", &RenderDevice::screenshotPic, py::arg("getAlpha") = false, py::arg("invertY") = true)
        .def("setObjectToWorldMatrix", &RenderDevice::setObjectToWorldMatrix)
        .def("setPolygonOffset", &RenderDevice::setPolygonOffset)
        .def("setDepthWrite", &RenderDevice::setDepthWrite)
    ;
}

void bind_texture(py::module &m) {

    py::class_<Texture::Encoding>(m, "Encoding") // Must be declared in front of Texture, because texture uses Encoding as a default argument
        .def(py::init<const ImageFormat*, FrameName, const float, const float>())
        .def(py::init<const ImageFormat*, FrameName, Color4, Color4>(), py::arg("fmt") = nullptr, py::arg("n") = FrameName::NONE, py::arg("rmf") = Color4::one(), py::arg("readaddsecond") = Color4::zero())
        .def("setFmt", [](Texture::Encoding& enc, ImageFormat* fmt){enc.format = fmt;}) 
        .def(py::init([](){
            return Texture::Encoding();
        }))
    ;

    py::enum_<Texture::Dimension>(m, "Dimension") // Must be declared in front of Texture
        .value("DIM_2D", Texture::DIM_2D)
        .value("DIM_2D_ARRAY", Texture::DIM_2D_ARRAY)
        .value("DIM_3D", Texture::DIM_3D)
        .value("DIM_2D_RECT", Texture::DIM_2D_RECT)
        .value("DIM_CUBE_MAP", Texture::DIM_CUBE_MAP)
        .value("DIM_CUBE_MAP_ARRAY", Texture::DIM_CUBE_MAP_ARRAY)
    ;

    py::class_<Texture, shared_ptr<Texture>>(m, "Texture")
        .def_static("whiteCube", &Texture::whiteCube, "()")
        .def_static("white", &Texture::white)
        .def_static("fromImage", [](const String& name, const shared_ptr<Image>& image){return Texture::fromImage(name, image, ImageFormat::AUTO(), Texture::DIM_2D, false);})
        .def_static("fromImage", [](std::string name, const shared_ptr<Image>& image){return Texture::fromImage(G3D::String(name), image, ImageFormat::AUTO(), Texture::DIM_2D, false);})
        .def_static("fromImage", &Texture::fromImage)
        .def_static("opaqueBlackIfNull", &Texture::opaqueBlackIfNull)
        .def_static("createEmpty", [](
            std::string                     name,
            int                             width,
            int                             height,
            const Texture::Encoding&        encoding        = Texture::Encoding(ImageFormat::RGBA8()),
            Texture::Dimension              dimension       = Texture::DIM_2D,
            bool                            generateMipMaps = false,
            int                             depth           = 1,
            int                             numSamples      = 1){
            return Texture::createEmpty(G3D::String(name), width, height, encoding, dimension, generateMipMaps, depth);
        })
        .def_static("createEmpty", &Texture::createEmpty, py::arg("name"), py::arg("width"), py::arg("height"), py::arg("encoding") = Texture::Encoding(ImageFormat::RGBA8()), py::arg("dimension") = Texture::DIM_2D, py::arg("generatemipmaps") = false, py::arg("depth") = 1, py::arg("numsamples") = 1)
        .def_static("getColorTex", [](GApp::Settings& s, String name, Vector2int32 hdrSize){
            return Texture::createEmpty(name, hdrSize.x, hdrSize.y, GLCaps::firstSupportedTexture(s.hdrFramebuffer.preferredColorFormats));
        })
        .def_static("getColorTex", [](GApp::Settings& s, std::string name, Vector2int32 hdrSize){
            return Texture::createEmpty(G3D::String(name), hdrSize.x, hdrSize.y, GLCaps::firstSupportedTexture(s.hdrFramebuffer.preferredColorFormats));
        })
        .def_static("getDepthTex", [](GApp::Settings& s, String name, Vector2int32 hdrSize){
            return Texture::createEmpty(name, hdrSize.x, hdrSize.y, GLCaps::firstSupportedTexture(s.hdrFramebuffer.preferredDepthFormats));
        })
        .def_static("getDepthTex", [](GApp::Settings& s, std::string name, Vector2int32 hdrSize){
            return Texture::createEmpty(G3D::String(name), hdrSize.x, hdrSize.y, GLCaps::firstSupportedTexture(s.hdrFramebuffer.preferredDepthFormats));
        })
    ;

}

void bind_oswindow_settings(py::module &m) {

    py::class_<OSWindow::Settings>(m, "OSWindowSettings")
        .def(py::init<>())
        .def_readwrite("width", &OSWindow::Settings::width)
        .def_readwrite("height", &OSWindow::Settings::height)
        .def_readwrite("api", &OSWindow::Settings::api)
    ;

    py::enum_<OSWindow::Settings::API>(m, "OSWindowSettingsAPI")
        .value("API_OPENGL_EGL", OSWindow::Settings::API_OPENGL_EGL)
        .value("API_OPENGL_ES", OSWindow::Settings::API_OPENGL_ES)
        .value("API_OPENGL", OSWindow::Settings::API_OPENGL)
        .value("API_VULKAN", OSWindow::Settings::API_VULKAN)
    ;
} 

void bind_oswindow(py::module &m) {

    py::class_<OSWindow>(m, "OSWindow")
        .def_static("create", &OSWindow::create, "settings")
        .def_static("pushGraphicsContext", &OSWindow::pushGraphicsContext, "OSWindow *window")
        .def_static("popGraphicsContext", &OSWindow::popGraphicsContext, "()")
        .def("framebuffer", &OSWindow::framebuffer, "()")
        .def("isIconified", &OSWindow::isIconified, "()")
        .def("popLoopBody", &OSWindow::popLoopBody, "()")
        .def("requiresMainLoop", &OSWindow::requiresMainLoop)
    ;
}

void bind_eglwindow(py::module &m) {

    py::class_<EGLWindow, OSWindow>(m, "EGLWindow")
        .def_static("create", &EGLWindow::create)
    ;
}

void bind_framebuffer(py::module &m) {

    py::class_<Framebuffer, std::shared_ptr<Framebuffer>>(m, "Framebuffer")
        .def_static("create", py::overload_cast<const String&>(&Framebuffer::create))
        .def_static("create", [](std::string name){return Framebuffer::create(G3D::String(name));})
        .def_static("create", py::overload_cast<const shared_ptr<Texture>&, const shared_ptr<Texture>&, const shared_ptr<Texture>&, const shared_ptr<Texture>&>(&Framebuffer::create), py::arg("tex0"), py::arg("tex1") = nullptr, py::arg("tex2") = nullptr, py::arg("tex3") = nullptr)
        .def("texture", py::overload_cast<const uint8>(&Framebuffer::texture))
        .def("texture", py::overload_cast<Framebuffer::AttachmentPoint>(&Framebuffer::texture))
        .def("vector2Bounds", &Framebuffer::vector2Bounds, "()")
        .def("width", &Framebuffer::width, "()")
        .def("height", &Framebuffer::height, "()")
        .def("set", py::overload_cast<Framebuffer::AttachmentPoint, const shared_ptr<Texture>&>(&Framebuffer::set))
        .def("resize", py::overload_cast<Vector2int32>(&Framebuffer::resize))
        .def("clear", &Framebuffer::clear)
    ;

    py::enum_<Framebuffer::AttachmentPoint>(m, "AttachmentPoint")
        .value("COLOR0", Framebuffer::COLOR0)
        .value("COLOR1", Framebuffer::COLOR1)
        .value("COLOR2", Framebuffer::COLOR2)
        .value("COLOR3", Framebuffer::COLOR3)
        .value("COLOR4", Framebuffer::COLOR4)
        .value("COLOR5", Framebuffer::COLOR5)
        .value("COLOR6", Framebuffer::COLOR6)
        .value("COLOR7", Framebuffer::COLOR7)
        .value("COLOR8", Framebuffer::COLOR8)
        .value("COLOR9", Framebuffer::COLOR9)
        .value("COLOR10", Framebuffer::COLOR10)
        .value("COLOR11", Framebuffer::COLOR11)
        .value("COLOR12", Framebuffer::COLOR12)
        .value("COLOR13", Framebuffer::COLOR13)
        .value("COLOR14", Framebuffer::COLOR14)
        .value("COLOR15", Framebuffer::COLOR15)
        .value("DEPTH", Framebuffer::DEPTH)
        .value("STENCIL", Framebuffer::STENCIL)
        .value("DEPTH_AND_STENCIL", Framebuffer::DEPTH_AND_STENCIL)
        .value("NO_ATTACHMENT", Framebuffer::NO_ATTACHMENT)
    ;
}

void bind_profiler(py::module &m) {

    py::class_<Profiler>(m, "Profiler")
        .def_static("nextFrame", &Profiler::nextFrame)
    ;
}

void bind_glcaps(py::module &m) {

    py::class_<GLCaps>(m, "GLCaps")
        .def_static("firstSupportedTexture", &GLCaps::firstSupportedTexture)
    ;
}

void bind_vertex_buffer(py::module &m) {

    py::class_<VertexBuffer, std::shared_ptr<VertexBuffer>>(m, "VertexBuffer")
        .def_static("cleanupAllVertexBuffers", &VertexBuffer::cleanupAllVertexBuffers)
    ;
}

void bind_init_gl_g3d(py::module &m) {
    m.def("initGLG3D", [](){initGLG3D(G3DSpecification());});
}

void bind_g3d_specification(py::module &m) {
    py::class_<G3DSpecification>(m, "G3DSpecification")
        .def(py::init<>())
    ;
}

void bind_gl_pixel_transfer_buffer(py::module &m) {
    py::class_<GLPixelTransferBuffer, shared_ptr<GLPixelTransferBuffer>>(m, "GLPixelTransferBuffer")
        .def_static("deleteAllBuffers", &GLPixelTransferBuffer::deleteAllBuffers)
    ;
}

void bind_g3d_gfx(py::module &m) {
    
    bind_render_device(m);
    bind_texture(m);
    bind_oswindow_settings(m);
    bind_oswindow(m);
    bind_eglwindow(m);
    bind_framebuffer(m);
    bind_profiler(m);
    bind_glcaps(m);
    bind_vertex_buffer(m);
    bind_init_gl_g3d(m);
    bind_g3d_specification(m);
    bind_gl_pixel_transfer_buffer(m);
}
}
