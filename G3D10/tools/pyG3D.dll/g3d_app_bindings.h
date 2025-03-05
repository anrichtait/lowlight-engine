#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include <G3D/G3D.h>
#include <pybind11/complex.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

namespace py = pybind11;
namespace G3D {

void bind_scene_loadoptions(py::module &m) {

    py::class_<Scene::LoadOptions>(m, "LoadOptions") 
        .def(py::init<>())
    ;
}

void bind_scene(py::module &m) {

    py::class_<Scene, shared_ptr<Scene>>(m, "Scene")
        .def_static("create", &Scene::create, py::arg("ao_sp"))
        .def("load", &Scene::load, py::arg("sceneName"), py::arg("loadOptions") = Scene::LoadOptions())
        .def("load", [](shared_ptr<Scene> scene, std::string name, Scene::LoadOptions lo){return scene->load(G3D::String(name), lo);}, py::arg("sceneName"), py::arg("loadOptions") = Scene::LoadOptions())
        .def("loadRBXL", &Scene::loadRBXL, py::arg("sceneName"), py::arg("loadOptions") = Scene::LoadOptions(), py::arg("modelGroupings") = false)
        .def("loadRBXL", [](shared_ptr<Scene> scene, std::string name, Scene::LoadOptions lo, bool modelGroupings){return scene->loadRBXL(G3D::String(name), lo, modelGroupings);}, py::arg("sceneName"), py::arg("loadOptions") = Scene::LoadOptions(), py::arg("modelGroupings") = false)
        .def("visualize", &Scene::visualize, "RenderDevice* rd, const shared_ptr<Entity>& selectedEntity, const Array<shared_ptr<Surface> >& allSurfaces, const SceneVisualizationSettings& settings, const shared_ptr<Camera>& camera")
        .def("insert", py::overload_cast<const shared_ptr<Entity>&>(&Scene::insert))
        .def("insert", py::overload_cast<const shared_ptr<Model>&>(&Scene::insert))
        .def("lightingEnvironment", py::overload_cast<>(&Scene::lightingEnvironment), "()")
        .def("clear", &Scene::clear, "()")
        .def("entity", &Scene::entity, "const String &name")
        .def("entity", [](shared_ptr<Scene> scene, std::string name){return scene->entity(G3D::String(name));}, "const String &name")
        .def("getEntityArray", [](shared_ptr<Scene> scene, Array<shared_ptr<Entity>>& arr){scene->getEntityArray(arr);})
        .def("onPose", &Scene::onPose)
        .def("appendSceneSearchPaths", &Scene::appendSceneSearchPaths)
        .def("defaultCamera", &Scene::defaultCamera)
        .def("name", &Scene::name)
        .def("modelTable", &Scene::modelTable)
    ;
}

void bind_entity(py::module &m) {

    py::class_<Entity, std::shared_ptr<Entity>>(m, "Entity")
        .def("setShouldBeSaved", &Entity::setShouldBeSaved)
        .def("getLastBounds", [](std::shared_ptr<Entity> e, AABox& b){return e->getLastBounds(b);})
        .def("name", &Entity::name)
        .def("setTrack", &Entity::setTrack)
    ;

    py::class_<Entity::Track, std::shared_ptr<Entity::Track>>(m, "Track")
    ;
}

void bind_camera(py::module &m) {

    py::class_<Camera, Entity, shared_ptr<Camera>>(m, "Camera")
        .def_static("create", py::overload_cast<const String&>(&Camera::create)) // dependent on c++14+
        .def_static("create", [](std::string name){return Camera::create(G3D::String(name));})
        .def("lookAt", &Camera::lookAt, py::arg("position"), py::arg("up") = Vector3::unitY())
        .def("setPosition", &Camera::setPosition)
        .def("setFrame", &Camera::setFrame, "(cframe, bool=true)")
        .def("projection", py::overload_cast<>(&Camera::projection), "()")
        .def("setFieldOfViewAngle", &Camera::setFieldOfViewAngle, "(float radians)")
        .def("motionBlurSettings", py::overload_cast<>(&Camera::motionBlurSettings), "()")
        .def("depthOfFieldSettings", py::overload_cast<>(&Camera::depthOfFieldSettings), "()")
        .def("filmSettings", py::overload_cast<>(&Camera::filmSettings), "()")
        .def("onPose", &Camera::onPose)
        .def("frame", &Camera::frame)
        .def("jitterMotion", &Camera::jitterMotion)
        .def("copyParametersFrom", &Camera::copyParametersFrom)
    ;
}

void bind_projection(py::module &m) {

    py::class_<Projection>(m, "Projection") 
        .def(py::init<>())
        .def(py::init<const Any&>())
        .def("setFarPlaneZ", &Projection::setFarPlaneZ, "float")
        .def("setNearPlaneZ", &Projection::setNearPlaneZ, "float")
        .def("setFieldOfViewAngleDegrees", &Projection::setFieldOfViewAngleDegrees, "float")
        .def("setFieldOfViewDirection", [](Projection& proj, FOVDirection::Value v) {proj.setFieldOfViewDirection(v);})//&Projection::setFieldOfViewDirection, "fovdirection")
        .def("toAny", &Projection::toAny, "()")
        .def("fieldOfViewAngle", &Projection::fieldOfViewAngle)
    ;
}

void bind_lighting_environment(py::module &m) {

    py::class_<LightingEnvironment>(m, "LightingEnvironment")
        .def(py::init<>())
        .def_readonly("environmentMapArray", &LightingEnvironment::environmentMapArray)
        .def_readonly("ambientOcclusionSettings", &LightingEnvironment::ambientOcclusionSettings)
        .def_readwrite("ambientOcclusion", &LightingEnvironment::ambientOcclusion)
    ;
}

void bind_ambient_occlusion(py::module &m) {

    py::class_<AmbientOcclusion, shared_ptr<AmbientOcclusion>>(m, "AmbientOcclusion") 
        .def_static("create", &AmbientOcclusion::create, py::arg("name") = String("G3D::AmbientOcclusion"))
        .def_static("create", [](std::string name){AmbientOcclusion::create(G3D::String(name));}, py::arg("name") = "G3D::AmbientOcclusion")
    ;

    py::class_<AmbientOcclusionSettings>(m, "ambientOcclusionSettings")
        .def("extendGBufferSpecification", &AmbientOcclusionSettings::extendGBufferSpecification)
        .def_readonly("enabled", &AmbientOcclusionSettings::enabled)
    ;
}

void bind_scene_visualization_settings(py::module &m) {

    py::class_<SceneVisualizationSettings>(m, "SceneVisualizationSettings")
        .def(py::init<>())
    ;
}

void bind_clean_geometry_settings(py::module &m) {

    py::class_<ArticulatedModel::CleanGeometrySettings>(m, "CleanGeometrySettings")
        .def(py::init<>())
        .def(py::init<const Any&>())
        .def_property("allowVertexMerging",
            [](const ArticulatedModel::CleanGeometrySettings& obj) {
                return obj.allowVertexMerging;
            },
            [](ArticulatedModel::CleanGeometrySettings& obj, bool val) {
                obj.allowVertexMerging = val;
            });
        ;
    ;
}

void bind_articulated_model_specification(py::module &m) {
    
    py::class_<ArticulatedModel::Specification>(m, "ArticulatedModelSpecification") //specifcally for articulatedmodels. used by articulatedmodel.create
        .def(py::init<>())
        .def("toAny", &ArticulatedModel::Specification::toAny, "()")
        .def_property("filename",
                [](const ArticulatedModel::Specification& obj) {
                    return obj.filename;
                },
                [](ArticulatedModel::Specification& obj, String newFilename) {
                    obj.filename = newFilename;
                }
        );
    ;
}

void bind_model(py::module &m) {

    py::class_<Model, std::shared_ptr<Model>>(m, "Model")
    ;

    // Lazy ptr not supported by pybind by default
    py::class_<lazy_ptr<Model>>(m, "LazyPtrModel")
        .def(py::init<>())
        .def(py::init<std::shared_ptr<Model>>())
        .def("name", [](lazy_ptr<Model>& ptr){
            auto articulated_ptr = std::dynamic_pointer_cast<ArticulatedModel>(ptr.resolve());
            if (articulated_ptr) {
                return articulated_ptr->name();
            } else {
                throw std::runtime_error("Dereferencing a null pointer or invalid cast");
            }}, "()")
        .def("saveOBJ", [](lazy_ptr<Model>& ptr, const String &filename){
            auto articulated_ptr = std::dynamic_pointer_cast<ArticulatedModel>(ptr.resolve());
            if (articulated_ptr) {
                return articulated_ptr->saveOBJ(filename);
            } else {
                throw std::runtime_error("Dereferencing a null pointer or invalid cast");
            }})
        .def("saveOBJ", [](lazy_ptr<Model>& ptr, std::string &filename){
            auto articulated_ptr = std::dynamic_pointer_cast<ArticulatedModel>(ptr.resolve());
            if (articulated_ptr) {
                return articulated_ptr->saveOBJ(G3D::String(filename));
            } else {
                throw std::runtime_error("Dereferencing a null pointer or invalid cast");
            }})
        .def("countTrianglesAndVertices", [](lazy_ptr<Model>& ptr, int& tri, int& vert){
            auto articulated_ptr = std::dynamic_pointer_cast<ArticulatedModel>(ptr.resolve());
            if (articulated_ptr) {
                return articulated_ptr->countTrianglesAndVertices(tri, vert);
            } else {
                throw std::runtime_error("Dereferencing a null pointer or invalid cast");
            }})
        .def("countVertices", [](lazy_ptr<Model>& ptr){
            auto articulated_ptr = std::dynamic_pointer_cast<ArticulatedModel>(ptr.resolve());
            if (articulated_ptr) {
                int tri = 9;
                int vert = 10;
                articulated_ptr->countTrianglesAndVertices(tri, vert);
                return vert;
            } else {
                throw std::runtime_error("Dereferencing a null pointer or invalid cast");
            }})
    ;
}

void bind_articulated_model(py::module &m) {

    py::class_<ArticulatedModel, Model, shared_ptr<ArticulatedModel>>(m, "ArticulatedModel")
        .def_static("create", &ArticulatedModel::create, py::arg("spec"), py::arg("name") = G3D::String(""))
        .def_static("create", [](ArticulatedModel::Specification& spec, std::string name){ArticulatedModel::create(spec, G3D::String(name));}, py::arg("spec"), py::arg("name") = "")
        .def("cleanGeometry", &ArticulatedModel::cleanGeometry, "(cleangeometrysettings)")
        .def("name", &ArticulatedModel::name, "()")
        .def("saveOBJ", &ArticulatedModel::saveOBJ, "const String &filename")
        .def("countTrianglesAndVertices", &ArticulatedModel::countTrianglesAndVertices)
        .def("countVertices", [](shared_ptr<ArticulatedModel>& model){
            int tri = 9;
            int vert = 10;
            model->countTrianglesAndVertices(tri, vert);
            return vert;
        })
    ;
}

void bind_shadow_map(py::module &m) {

    py::class_<ShadowMap, shared_ptr<ShadowMap>>(m, "ShadowMap") //not supported in linux?
        .def_static("create", &ShadowMap::create, " (const String& name, Vector2int16 desiredSize = Vector2int16(1024, 1024), const VSMSettings& vsmSettings = VSMSettings());")
        .def("setBias", &ShadowMap::setBias, "(float)")
    ;
}

void bind_visible_entity(py::module &m) {

    py::class_<VisibleEntity, Entity, std::shared_ptr<VisibleEntity>>(m, "VisibleEntity")
        .def_static("create", [](const String &name, shared_ptr<Scene> scene, const shared_ptr<Model>& model){ return VisibleEntity::create(G3D::String(name), scene.get(), model);}) // Add default arguments
        .def_static("create", [](std::string name, shared_ptr<Scene> scene, const shared_ptr<Model>& model){ return VisibleEntity::create(G3D::String(name), scene.get(), model);})
        .def("setVisible", &VisibleEntity::setVisible, "bool")
        .def_static("fromEntity", [](std::shared_ptr<Entity> entity){ return std::dynamic_pointer_cast<VisibleEntity>(entity); })
    ; 
}

void bind_skybox(py::module &m) {

    py::class_<Skybox, Entity, shared_ptr<Skybox>>(m, "Skybox")
        .def_static("create", [](const String &name, shared_ptr<Scene> scene, const Array<shared_ptr<Texture>> &k, const Array<SimTime>& t, SimTime f, SplineExtrapolationMode::Value e, bool canChange, bool shouldBeSaved){ return Skybox::create("skybox", scene.get(), k, t, f, e, canChange, shouldBeSaved);})
        .def_static("create", [](std::string &name, shared_ptr<Scene> scene, const Array<shared_ptr<Texture>> &k, const Array<SimTime>& t, SimTime f, SplineExtrapolationMode::Value e, bool canChange, bool shouldBeSaved){ return Skybox::create("skybox", scene.get(), k, t, f, e, canChange, shouldBeSaved);})
        .def("keyframeArray", &Skybox::keyframeArray)
    ;
}

void bind_light(py::module &m) {

    py::class_<Light, VisibleEntity, shared_ptr<Light>>(m, "Light")
        .def_static("spotTarget", &Light::spotTarget, py::arg("name"), py::arg("pos"), py::arg("target"), py::arg("halfAngleRadians"), py::arg("color"), py::arg("constAtt") = 0.01f, py::arg("linAtt") = 0, py::arg("quadAtt") = 1.0f, py::arg("shadowsEnabled") = true, py::arg("shadowMapRes") = 2048)
        .def_static("spotTarget", [](
            std::string name,
            const Point3& pos,
            const Point3& target,
            float halfAngleRadians,
            const Color3& color,
            float constAtt = 0.01f,
            float linAtt = 0,
            float quadAtt = 1.0f,
            bool  shadowsEnabled = true,
            int  shadowMapRes = 2048){
                return Light::spotTarget(G3D::String(name), pos, target, halfAngleRadians, color, constAtt, linAtt, quadAtt, shadowsEnabled);
            }) // Have yet to test if this works
        .def_static("directional", &Light::directional, py::arg("name"), py::arg("toLight"), py::arg("color"), py::arg("shadowsEnabled") = true, py::arg("shadowMapRes") = 2048)
        .def_static("directional", [](
            std::string name,
            const Vector3& pos,
            const Color3& color,
            bool  shadowsEnabled = true,
            int  shadowMapRes = 2048){
                return Light::directional(G3D::String(name), pos, color, shadowsEnabled);
            })
        .def_static("point", &Light::point, py::arg("name"), py::arg("pos"), py::arg("color"), py::arg("constAtt") = 0.01f, py::arg("linAtt") = 0, py::arg("quadAtt") = 1.0f, py::arg("shadowsEnabled") = true, py::arg("shadowMapRes") = 2048)
        .def_static("point", [](
            std::string name,
            const Point3& pos,
            const Color3& color,
            float constAtt = 0.01f,
            float linAtt = 0,
            float quadAtt = 1.0f,
            bool  shadowsEnabled = true,
            int  shadowMapRes = 2048){
                return Light::point(G3D::String(name), pos, color, constAtt, linAtt, quadAtt, shadowsEnabled);
            })
        .def_static("area", &Light::area, py::arg("name"), py::arg("cframe"), py::arg("halfAngleRadians"), py::arg("color"), py::arg("extent"), py::arg("constAtt") = 0.01f, py::arg("linAtt") = 0, py::arg("quadAtt") = 1.0f, py::arg("shadowsEnabled") = true, py::arg("shadowMapRes") = 2048)
        .def_static("area", [](
            std::string name,
            const CFrame& cframe,
            float halfAngleRadians,
            const Color3& color,
            Vector2 extent,
            float constAtt = 0.01f,
            float linAtt = 0,
            float quadAtt = 1.0f,
            bool  shadowsEnabled = true,
            int  shadowMapRes = 2048){
                return Light::area(G3D::String(name), cframe, halfAngleRadians, color, extent, constAtt, linAtt, quadAtt, shadowsEnabled);
            })
        .def_static("spot", &Light::spot, py::arg("name"), py::arg("cframe"), py::arg("pointDirection"), py::arg("halfAngleRadians"), py::arg("color"), py::arg("constAtt") = 0.01f, py::arg("linAtt") = 0, py::arg("quadAtt") = 1.0f, py::arg("shadowsEnabled") = true, py::arg("shadowMapRes") = 2048)
        .def_static("spotTarget", [](
            std::string name,
            const Point3& pos,
            const Vector3& pointDirection,
            float halfAngleRadians,
            const Color3& color,
            float constAtt = 0.01f,
            float linAtt = 0,
            float quadAtt = 1.0f,
            bool  shadowsEnabled = true,
            int  shadowMapRes = 2048){
                return Light::spot(G3D::String(name), pos, pointDirection, halfAngleRadians, color, constAtt, linAtt, quadAtt, shadowsEnabled);
            })
        .def("shadowMap", &Light::shadowMap, "()")
        .def("setShadowsEnabled", &Light::setShadowsEnabled)
        .def("setSpotHardness", &Light::setSpotHardness)
    ;
}

void bind_gbuffer_specification(py::module &m) {

    py::class_<GBuffer::Specification>(m, "GBufferSpecification")
        .def(py::init<>())
        .def_readwrite("depthEncoding", &GBuffer::Specification::depthEncoding)
        .def("init", [](GBuffer::Specification& s){
            s.encoding[GBuffer::Field::CS_NORMAL]        = Texture::Encoding(ImageFormat::RGB10A2(), FrameName::CAMERA, 2.0f, -1.0f);
            s.encoding[GBuffer::Field::DEPTH_AND_STENCIL]     = ImageFormat::DEPTH32();
            s.depthEncoding = DepthEncoding::HYPERBOLIC;
        })
        .def("deferredShading", [](GBuffer::Specification& s){
            s.encoding[GBuffer::Field::CS_FACE_NORMAL].format = nullptr;
            s.encoding[GBuffer::Field::EMISSIVE]           = 
                Texture::Encoding(ImageFormat::R11G11B10F(), FrameName::NONE, 3.0f, 0.0f);
            s.encoding[GBuffer::Field::LAMBERTIAN]         = ImageFormat::RGB8();
            s.encoding[GBuffer::Field::GLOSSY]             = ImageFormat::RGBA8();
        })
        .def("sanityCheck", [](GBuffer::Specification& s){ // Function for verifying that encoding indices are being preserved
            for (int f = 0; f < GBuffer::Field::COUNT; ++f) {
                const ImageFormat* fmt = s.encoding[f].format;
                if (notNull(fmt)) {
                    std::cout << f <<std::endl;
                }
            }
            std::cout << "done" << std::endl;
        })
    ;
}

void bind_gbuffer(py::module &m) {

    py::class_<GBuffer, std::shared_ptr<GBuffer>>(m, "GBuffer")
        .def_static("create", &GBuffer::create, py::arg("spec"), py::arg("name") = String("G3D::Buffer"))
        .def_static("create", [](GBuffer::Specification& spec, std::string name){return GBuffer::create(spec, G3D::String(name));})
        .def("setSpecification", py::overload_cast<const GBuffer::Specification&>(&GBuffer::setSpecification), "spec")
        .def("resize", py::overload_cast<Vector2int32, int>(&GBuffer::resize), py::arg("size"), py::arg("depth") = 1)
        .def("resize", py::overload_cast<int, int, int>(&GBuffer::resize), "width, height, depth=1")

        .def("prepare", py::overload_cast<RenderDevice*, const shared_ptr<Camera>&, float, float, Vector2int16, Vector2int16>(&GBuffer::prepare))
        .def("prepare", py::overload_cast<RenderDevice*, float, float, Vector2int16, Vector2int16>(&GBuffer::prepare))
        .def("width", &GBuffer::width, "()")
        .def("texture", [](std::shared_ptr<GBuffer> gbuffer, GBuffer::Field::Value f){return gbuffer->texture(f);})
        .def("texture", &GBuffer::texture)
    ;

    py::enum_<GBuffer::Field::Value>(m, "GBufferField")
        .value("WS_NORMAL", GBuffer::Field::WS_NORMAL)
        .value("CS_NORMAL", GBuffer::Field::CS_NORMAL)
        .value("WS_FACE_NORMAL", GBuffer::Field::WS_FACE_NORMAL)
        .value("CS_FACE_NORMAL", GBuffer::Field::CS_FACE_NORMAL)
        .value("WS_POSITION", GBuffer::Field::WS_POSITION)
        .value("CS_POSITION", GBuffer::Field::CS_POSITION)
        .value("LAMBERTIAN", GBuffer::Field::LAMBERTIAN)
        .value("GLOSSY", GBuffer::Field::GLOSSY)
        .value("TRANSMISSIVE", GBuffer::Field::TRANSMISSIVE)
        .value("EMISSIVE", GBuffer::Field::EMISSIVE)
        .value("CS_POSITION_CHANGE", GBuffer::Field::CS_POSITION_CHANGE)
        .value("SS_POSITION_CHANGE", GBuffer::Field::SS_POSITION_CHANGE)
        .value("CS_Z", GBuffer::Field::CS_Z)
        .value("DEPTH_AND_STENCIL", GBuffer::Field::DEPTH_AND_STENCIL)
        .value("TS_NORMAL", GBuffer::Field::TS_NORMAL)
        .value("SVO_POSITION", GBuffer::Field::SVO_POSITION)
        .value("FLAGS", GBuffer::Field::FLAGS)
        .value("SVO_COVARIANCE_MAT1", GBuffer::Field::SVO_COVARIANCE_MAT1)
        .value("SVO_COVARIANCE_MAT2", GBuffer::Field::SVO_COVARIANCE_MAT2)
        .value("TEXCOORD0", GBuffer::Field::TEXCOORD0)
        .value("COUNT", GBuffer::Field::COUNT)
    ;
}

void bind_gapp_settings(py::module &m) {

    py::class_<GApp::Settings>(m, "GAppSettings")
        .def(py::init<>())
        .def_readonly("window", &GApp::Settings::window)
        .def_readonly("debugFontName", &GApp::Settings::debugFontName)
        .def_readwrite("writeLicenseFile", &GApp::Settings::writeLicenseFile)
        .def_readwrite("useDeveloperTools", &GApp::Settings::useDeveloperTools)
        .def_readwrite("dataDir", &GApp::Settings::dataDir)
        .def_readwrite("dataDirs", &GApp::Settings::dataDirs)
        .def_readonly("screenCapture", &GApp::Settings::screenCapture)
        .def_readonly("hdrFramebuffer", &GApp::Settings::hdrFramebuffer)
        .def_readwrite("argArray", &GApp::Settings::argArray)
        .def_readwrite("renderer", &GApp::Settings::renderer)
    ;

    py::enum_<GApp::SubmitToDisplayMode::Value>(m, "SubmitToDisplayMode")
        .value("EXPLICIT", GApp::SubmitToDisplayMode::EXPLICIT)
        .value("MAXIMIZE_THROUGHPUT", GApp::SubmitToDisplayMode::MAXIMIZE_THROUGHPUT)
        .value("BALANCE", GApp::SubmitToDisplayMode::BALANCE)
        .value("MINIMIZE_LATENCY", GApp::SubmitToDisplayMode::MINIMIZE_LATENCY)
    ;

    py::enum_<GApp::Action>(m, "Action")
        .value("ACTION_QUIT", GApp::ACTION_QUIT)
        .value("ACTION_NONE", GApp::ACTION_NONE)
        .value("ACTION_SHOW_CONSOLE", GApp::ACTION_SHOW_CONSOLE)
    ;

    py::class_<GApp::Settings::HDRFramebufferSettings>(m, "HDRFramebufferSettings")
        .def("hdrFramebufferSizeFromDeviceSize", &GApp::Settings::HDRFramebufferSettings::hdrFramebufferSizeFromDeviceSize, "(vector2int32)")
        .def_readonly("depthGuardBandThickness", &GApp::Settings::HDRFramebufferSettings::depthGuardBandThickness)
        .def_readonly("colorGuardBandThickness", &GApp::Settings::HDRFramebufferSettings::colorGuardBandThickness)
        .def_readonly("preferredColorFormats", &GApp::Settings::HDRFramebufferSettings::preferredColorFormats)
        .def_readonly("preferredDepthFormats", &GApp::Settings::HDRFramebufferSettings::preferredDepthFormats)
        .def("trimBandThickness", &GApp::Settings::HDRFramebufferSettings::trimBandThickness)
    ;

    py::class_<GApp::Settings::RendererSettings>(m, "RendererSettings")
        .def(py::init<>())
        .def_readonly("deferredShading", &GApp::Settings::RendererSettings::deferredShading)
        .def_readonly("orderIndependentTransparency", &GApp::Settings::RendererSettings::orderIndependentTransparency)
        .def_static("hasFactory", [](GApp::Settings::RendererSettings& r){if(r.factory == nullptr) return false; return true;})
        .def_static("factory", [](GApp::Settings::RendererSettings& r){return r.factory();})
    ;
}

void bind_widget_manager(py::module &m) {

    py::class_<WidgetManager, std::shared_ptr<WidgetManager>>(m, "WidgetManager")
        .def_static("create", &WidgetManager::create, "oswindow *")
        .def("remove", &WidgetManager::remove, "shared_ptr<widget>")
        .def("add", &WidgetManager::add, "shared_ptr<widget>")
        .def("setFocusedWidget", &WidgetManager::setFocusedWidget, py::arg("module"), py::arg("bringToFront") = true)
    ;
}

void bind_user_input(py::module &m) {

    py::class_<UserInput>(m, "UserInput")
        .def_static("new", [](OSWindow *window){ return new UserInput(window);})
    ;
}

void bind_marker_entity(py::module &m) {

    py::class_<MarkerEntity, Entity, std::shared_ptr<MarkerEntity>>(m, "MarkerEntity")
        .def_static("create", py::overload_cast<const String&>(&MarkerEntity::create))
        .def_static("create", [](std::string name){return MarkerEntity::create(G3D::String(name));})
        .def_static("create", [](const String& str, Scene *scene, const Array<Box>& boxarr, const Color3& color, const CFrame& cframe, const shared_ptr<Entity::Track> track, bool canchange, bool shouldbesaved){ return MarkerEntity::create(str, scene, boxarr, color, cframe, track, canchange, shouldbesaved);},
            py::arg("name"),
            py::arg("scene"),
            py::arg("osBoxArray") =  Array<Box>(Box(Point3(-0.25f, -0.25f, -0.25f), Point3(0.25f, 0.25f, 0.25f))),
            py::arg("color") = Color3::white(),
            py::arg("frame") = CFrame(),
            py::arg("track") = std::shared_ptr<Entity::Track>(),
            py::arg("canChange") = true,
            py::arg("shouldBeSaved") = true
        )
        .def_static("create", [](std::string str, Scene *scene, const Array<Box>& boxarr, const Color3& color, const CFrame& cframe, const shared_ptr<Entity::Track> track, bool canchange, bool shouldbesaved){ return MarkerEntity::create(G3D::String(str), scene, boxarr, color, cframe, track, canchange, shouldbesaved);},
            py::arg("name"),
            py::arg("scene"),
            py::arg("osBoxArray") =  Array<Box>(Box(Point3(-0.25f, -0.25f, -0.25f), Point3(0.25f, 0.25f, 0.25f))),
            py::arg("color") = Color3::white(),
            py::arg("frame") = CFrame(),
            py::arg("track") = std::shared_ptr<Entity::Track>(),
            py::arg("canChange") = true,
            py::arg("shouldBeSaved") = true
        )
        .def("setShouldBeSaved", &MarkerEntity::setShouldBeSaved)
    ;
}

void bind_widget(py::module &m) {

    py::class_<Widget, std::shared_ptr<Widget>>(m, "Widget")
    ;
}

void bind_manipulator(py::module &m) {

    py::class_<Manipulator, Widget, std::shared_ptr<Manipulator>>(m, "Manipulator")
    ;
}

void bind_first_person_manipulator(py::module &m){
    py::class_<FirstPersonManipulator, Manipulator, std::shared_ptr<FirstPersonManipulator>>(m, "FirstPersonManipulator")
        .def_static("create", &FirstPersonManipulator::create)
        .def("onUserInput", &FirstPersonManipulator::onUserInput)
        .def("setMoveRate", &FirstPersonManipulator::setMoveRate)
        .def("setPosition", &FirstPersonManipulator::setPosition)
        .def("lookAt", &FirstPersonManipulator::lookAt)
        .def("setMouseMode", &FirstPersonManipulator::setMouseMode)
        .def("setEnabled", &FirstPersonManipulator::setEnabled)
        .def("reset", &FirstPersonManipulator::reset)
        .def("translation", &FirstPersonManipulator::translation)
    ;

    py::enum_<FirstPersonManipulator::MouseMode>(m, "MouseMode")
        .value("MOUSE_DIRECT", FirstPersonManipulator::MouseMode::MOUSE_DIRECT)
        .value("MOUSE_DIRECT_RIGHT_BUTTON", FirstPersonManipulator::MouseMode::MOUSE_DIRECT_RIGHT_BUTTON)
        .value("MOUSE_SCROLL_AT_EDGE", FirstPersonManipulator::MouseMode::MOUSE_SCROLL_AT_EDGE)
        .value("MOUSE_PUSH_AT_EDGE", FirstPersonManipulator::MouseMode::MOUSE_PUSH_AT_EDGE)
    ;
}

void bind_gfont(py::module &m) {

    py::class_<GFont, std::shared_ptr<GFont>>(m, "GFont")
        .def_static("loadFont", [](const String& fontName) {
            std::shared_ptr<GFont> loadedFont;
            logPrintf("Entering GApp::loadFont(\"%s\")\n", fontName.c_str());
            const String& filename = System::findDataFile(fontName);
            logPrintf("Found \"%s\" at \"%s\"\n", fontName.c_str(), filename.c_str());
            if (FileSystem::exists(filename)) {
                loadedFont = GFont::fromFile(filename);
            } else {
                logPrintf(
                    "Warning: G3D::GApp could not load font \"%s\".\n"
                    "This may be because the G3D::GApp::Settings::dataDir was not\n"
                    "properly set in main().\n",
                    filename.c_str());
            }
            logPrintf("Done GApp::loadFont(...)\n");
            return loadedFont;
        })
        .def_static("fromFile", &GFont::fromFile)
    ;
}

void bind_gconsole(py::module &m) {
    
    py::class_<GConsole::Settings>(m, "GConsoleSettings") 
        .def(py::init<>())
        .def_readwrite("backgroundColor", &GConsole::Settings::backgroundColor)
    ;

    py::class_<GConsole, Widget, std::shared_ptr<GConsole>>(m, "GConsole") 
        .def_static("create", []( const shared_ptr<GFont>& f, const GConsole::Settings& s) {return GConsole::create(f, s);})
        .def("setActive", &GConsole::setActive)
    ;
}

void bind_film(py::module &m) {

    py::class_<Film, std::shared_ptr<Film>>(m, "Film")
        .def_static("create", &Film::create)
        .def("exposeAndRender", py::overload_cast<
         RenderDevice*, 
         const FilmSettings&,
         const shared_ptr<Texture>&,
         int, 
         int,
         const shared_ptr<Texture>&,
         const Vector2
         > (&Film::exposeAndRender), py::arg("rd"), py::arg("settings"), py::arg("input"), py::arg("sourceTrimBandThickness"), py::arg("sourceDepthBandThickness"), py::arg("screenSpaceMotion") = nullptr, py::arg("jitterMotion") = Vector2::zero())
    ;

    py::class_<FilmSettings> (m, "FilmSettings")
        .def(py::init<>())
        .def("extendGBufferSpecification", &FilmSettings::extendGBufferSpecification)
    ;
}

void bind_screen_capture(py::module &m){

    py::class_<GApp::Settings::ScreenCaptureSettings>(m, "ScreenCaptureSettings")
        .def_readonly("addFilesToSourceControl", &GApp::Settings::ScreenCaptureSettings::addFilesToSourceControl)
        .def_readonly("outputDirectory", &GApp::Settings::ScreenCaptureSettings::outputDirectory)
        .def_readonly("includeAppRevision", &GApp::Settings::ScreenCaptureSettings::includeAppRevision)
        .def_readonly("includeG3DRevision", &GApp::Settings::ScreenCaptureSettings::includeG3DRevision)
        .def_readonly("filenamePrefix", &GApp::Settings::ScreenCaptureSettings::filenamePrefix)
    ;

    py::class_<ScreenCapture>(m, "ScreenCapture")
        .def_static("checkAppScmRevision", &ScreenCapture::checkAppScmRevision, "output dir")
    ;
}

void bind_gui_theme(py::module &m){

    py::class_<GuiTheme, std::shared_ptr<GuiTheme>>(m, "GuiTheme")
        .def_static("fromFile", &GuiTheme::fromFile, py::arg("filename"), py::arg("fallbackfont"), py::arg("fallbacksize") = 11, py::arg("fallbackcolor") = Color4(Color3::black(), 1.0), py::arg("fallbackoutlinecolor") = Color4::clear())
    ;

    py::enum_<GuiTheme::WindowStyle>(m, "WindowStyle")
        .value("NORMAL_WINDOW_STYLE", GuiTheme::WindowStyle::NORMAL_WINDOW_STYLE)
        .value("TOOL_WINDOW_STYLE", GuiTheme::WindowStyle::TOOL_WINDOW_STYLE)
        .value("DIALOG_WINDOW_STYLE", GuiTheme::WindowStyle::DIALOG_WINDOW_STYLE)
        .value("DRAWER_WINDOW_STYLE", GuiTheme::WindowStyle::DRAWER_WINDOW_STYLE)
        .value("MENU_WINDOW_STYLE", GuiTheme::WindowStyle::MENU_WINDOW_STYLE)
        .value("PANEL_WINDOW_STYLE", GuiTheme::WindowStyle::PANEL_WINDOW_STYLE)
        .value("PARTIAL_DISAPPEARING_STYLE", GuiTheme::WindowStyle::PARTIAL_DISAPPEARING_STYLE)
        .value("FULL_DISAPPEARING_STYLE", GuiTheme::WindowStyle::FULL_DISAPPEARING_STYLE)
        .value("NO_WINDOW_STYLE", GuiTheme::WindowStyle::NO_WINDOW_STYLE)
    ;
}

void bind_gui_window(py::module &m) {

    py::class_<GuiWindow, std::shared_ptr<GuiWindow>>(m, "GuiWindow")
        .def_static("create", &GuiWindow::create)
        .def("pane", py::overload_cast<>(&GuiWindow::pane), "()")
    ;

    py::enum_<GuiWindow::CloseAction>(m, "CloseAction")
        .value("NO_CLOSE", GuiWindow::CloseAction::NO_CLOSE)
        .value("IGNORE_CLOSE", GuiWindow::CloseAction::IGNORE_CLOSE)
        .value("HIDE_ON_CLOSE", GuiWindow::CloseAction::HIDE_ON_CLOSE)
        .value("REMOVE_ON_CLOSE", GuiWindow::CloseAction::REMOVE_ON_CLOSE)
    ;
}

void bind_depth_of_field(py::module &m) {
    
    py::enum_<DepthOfField::DebugOption>(m, "DebugOption")
        .value("NONE", DepthOfField::NONE)
        .value("SHOW_COC", DepthOfField::SHOW_COC)
        .value("SHOW_REGION", DepthOfField::SHOW_REGION)
        .value("SHOW_NEAR", DepthOfField::SHOW_NEAR)
        .value("SHOW_BLURRY", DepthOfField::SHOW_BLURRY)
        .value("SHOW_INPUT", DepthOfField::SHOW_INPUT)
        .value("SHOW_MID_AND_FAR", DepthOfField::SHOW_MID_AND_FAR)
        .value("SHOW_SIGNED_COC", DepthOfField::SHOW_SIGNED_COC);
    ;

    py::class_<DepthOfField, std::shared_ptr<DepthOfField>>(m, "DepthOfField")
        .def_static("create", &DepthOfField::create, py::arg("name") = String("G3D::DepthOfField"))
        .def_static("create", &DepthOfField::create, py::arg("name") = String("G3D::DepthOfField"))
        .def("apply", &DepthOfField::apply, py::arg("rd"), py::arg("color"), py::arg("depth"), py::arg("camera"), py::arg("trimBandThickness"), py::arg("debugOption") = DepthOfField::NONE)
    ;

    py::class_<DepthOfFieldSettings> (m, "DepthOfFieldSettings")
        .def(py::init<>())
        .def("extendGBufferSpecification", &DepthOfFieldSettings::extendGBufferSpecification)
    ;
}

void bind_motion_blur(py::module &m) {

    py::class_<MotionBlur, std::shared_ptr<MotionBlur>>(m, "MotionBlur")
        .def_static("create", &MotionBlur::create, "()")
        .def("apply", &MotionBlur::apply, py::arg("rd"), py::arg("color"), py::arg("velocity"), py::arg("depth"), py::arg("camera"), py::arg("trimBandThickness"))
    ;

    py::class_<MotionBlurSettings> (m, "MotionBlurSettings")
        .def(py::init<>())
        .def("extendGBufferSpecification", &MotionBlurSettings::extendGBufferSpecification)
    ;
}

void bind_depth_encoding(py::module &m) {

    py::enum_<DepthEncoding::Value>(m, "DepthEncoding")
        .value("HYPERBOLIC", DepthEncoding::HYPERBOLIC)
        .value("LINEAR", DepthEncoding::LINEAR)
        .value("COMPLEMENTARY", DepthEncoding::COMPLEMENTARY)
    ;
}

void bind_renderer(py::module &m) {

    py::class_<Renderer, std::shared_ptr<Renderer>>(m, "Renderer")
        .def_static("tritreefunc", [](Scene* scene){return ([&]() -> decltype(auto) { return scene->tritree(); });})
        .def("render", 
        
        [](std::shared_ptr<Renderer>        renderer,
        RenderDevice*                       rd, 
        const shared_ptr<Camera>&           camera,
        const shared_ptr<Framebuffer>&      framebuffer,
        const shared_ptr<Framebuffer>&      depthPeelFramebuffer,
        LightingEnvironment&                lightingEnvironment,
        const shared_ptr<GBuffer>&          gbuffer, 
        const Array<shared_ptr<Surface>>&   allSurfaces,
        Scene *                             scene){ return renderer->render(rd, camera, framebuffer, depthPeelFramebuffer, lightingEnvironment, gbuffer, allSurfaces, [&]() -> decltype(auto) { return scene->tritree(); });})
    ;
}

void bind_default_renderer(py::module &m) {

    py::class_<DefaultRenderer, std::shared_ptr<DefaultRenderer>>(m, "DefaultRenderer")
        .def("create", &DefaultRenderer::create)
        .def_static("fromRenderer", [](std::shared_ptr<Renderer> renderer){ return std::dynamic_pointer_cast<DefaultRenderer>(renderer); })
        .def("setDeferredShading", &DefaultRenderer::setDeferredShading, "bool")
        .def("setOrderIndependentTransparency", &DefaultRenderer::setOrderIndependentTransparency)
        .def("render", &DefaultRenderer::render)
    ;
}

void bind_g3d_app(py::module &m){

    bind_scene_loadoptions(m);
    bind_scene(m);
    bind_entity(m);
    bind_camera(m);
    bind_projection(m);
    bind_lighting_environment(m);
    bind_ambient_occlusion(m);
    bind_scene_visualization_settings(m);
    bind_clean_geometry_settings(m);
    bind_articulated_model_specification(m);
    bind_model(m);
    bind_articulated_model(m);
    bind_shadow_map(m);
    bind_visible_entity(m);
    bind_skybox(m);
    bind_light(m);
    bind_gbuffer_specification(m);
    bind_gbuffer(m);
    bind_gapp_settings(m);
    bind_widget_manager(m);
    bind_user_input(m);
    bind_marker_entity(m);
    bind_widget(m);
    bind_manipulator(m);
    bind_first_person_manipulator(m);
    bind_gfont(m);
    bind_gconsole(m);
    bind_film(m);
    bind_screen_capture(m);
    bind_gui_theme(m);
    bind_gui_window(m);
    bind_depth_of_field(m);
    bind_motion_blur(m);
    bind_depth_encoding(m);
    bind_renderer(m);
    bind_default_renderer(m);
}
}
