/** \file App.cpp */
#include "App.h"

// Set to 1 to process a whole directory.
// Set to 0 to interactively debug.
#define BATCH_PROCESS 0

/* Parent of the input path. All subdirectories of this two levels down will be tested*/
static const String SOURCE_PATH = "input";

/* 1 = look in subdirectores of SOURCE_PATH. 2 = look in subdirectories of subdirectories */
static const int    SOURCE_DEPTH = 2;

static const String DEST_PATH = "output";

void App::render
   (const Source&                   source, 
    const Vector3&                  directionToLight, 
    const Radiance3&                lightRadiance, 
    const shared_ptr<Texture>&      environmentMap, 
    const shared_ptr<Framebuffer>&  destination) const {

    RenderDevice* rd = renderDevice;

    rd->push2D(destination); {
        Args args;
        source.color->setShaderArgs(args, "colorTexture.", Sampler::buffer());
        source.normal->setShaderArgs(args, "normalTexture.", Sampler::defaults());
        source.displacement->setShaderArgs(args, "displacementTexture.", Sampler::defaults());
        source.roughness->setShaderArgs(args, "roughnessTexture.", Sampler::buffer());
        source.metalness->setShaderArgs(args, "metalnessTexture.", Sampler::buffer());
        source.glossiness->setShaderArgs(args, "glossinessTexture.", Sampler::buffer());
        source.ambientOcclusion->setShaderArgs(args, "ambientOcclusionTexture.", Sampler::buffer());
        environmentMap->setShaderArgs(args, "environmentMapTexture.", Sampler::cubeMap());

        args.setUniform("directionToLight", directionToLight.direction());
        args.setUniform("lightRadiance", lightRadiance);
        args.setRect(destination->rect2DBounds());
        LAUNCH_SHADER("shade.*", args);
    } rd->pop2D();
}

///////////////////////////////////////////////////////////////////////

static shared_ptr<Texture> loadTexture(const String& directory, const Array<String>& suffixes, const ImageFormat* format, const shared_ptr<Texture>& defaultValue, bool generateMipMaps = false) {
    static const Array<String> fileTypes("png", "jpg", "tga", "tif");

    FileSystem::ListSettings listSettings;
    listSettings.caseSensitive = false;
    listSettings.directories = false;
    listSettings.files = true;
    listSettings.includeParentPath = true;
    listSettings.recursive = false;

    for (const String& suffix : suffixes) {
        for (const String& extension : fileTypes) {
            // Because we're running on windows, ignoring case is OK. Look for anything that matches 
            // the wildcards
            const String& filenameSpec = FilePath::concat(directory, "*") + suffix + "*." + extension;
            if (FileSystem::exists(filenameSpec)) {
                // Find the actual filename
                Array<String> result;
                FileSystem::list(filenameSpec, result, listSettings);

                alwaysAssertM(result.length() > 0, "No result");
                const String& filename = result[0];

                debugPrintf("Loaded %s\n", filename.c_str());
                return Texture::fromFile(filename, format, Texture::DIM_2D, generateMipMaps);
            }
        }
    }

    return defaultValue;
}

Source::Source(const String& directory) : name(directory) {
    static const Array<String> ambientOcclusionSuffixes("AmbientOcclusion", "AO", "Occlusion", "OCC");
    static const Array<String> displacementSuffixes("Displacement", "DISP", "Bump", "h", "Height");
    static const Array<String> normalSuffixes("Normal", "NORM", "n");
    static const Array<String> colorSuffixes("Diffuse", "Color", "Albedo", "BaseColor", "Base_Color", "Col", "Diff");
    static const Array<String> roughnessSuffixes("Roughness", "Spec", "Specular", "Rough");
    static const Array<String> metalnessSuffixes("Metalness", "Metallic", "Metal");
    static const Array<String> glossinessSuffixes("glossiness");

    static const shared_ptr<Texture> defaultNormal = Texture::create(Texture::Specification(Color4(0, 1, 0, 1)));

    ambientOcclusion = loadTexture(directory, ambientOcclusionSuffixes, ImageFormat::R8(),      Texture::white());
    displacement     = loadTexture(directory, displacementSuffixes,     ImageFormat::R8(),      Texture::opaqueBlack(), true);
    normal           = loadTexture(directory, normalSuffixes,           ImageFormat::RGB8(),    defaultNormal, true);
    color            = loadTexture(directory, colorSuffixes,            ImageFormat::SRGB8(),   Texture::white());
    roughness        = loadTexture(directory, roughnessSuffixes,        ImageFormat::R8(),      Texture::opaqueBlack());
    metalness        = loadTexture(directory, metalnessSuffixes,        ImageFormat::R8(),      Texture::opaqueBlack());
    glossiness       = loadTexture(directory, glossinessSuffixes,       ImageFormat::R8(),      Texture::opaqueBlack());
}


///////////////////////////////////////////////////////////////////////


App::App(const GApp::Settings& settings) : GApp(settings) {}


static Source source;

void App::onInit() {
    GApp::onInit();
    renderDevice->setSwapBuffersAutomatically(true);

    makeGUI();

    // The provided sample textures are 2048x2048, but one possible use is viewing a subset on 1920x1080 screens
    //m_destination = Framebuffer::create(Texture::createEmpty("m_destination", 2048, 2048, ImageFormat::RGB32F()));
    m_destination = Framebuffer::create(Texture::createEmpty("m_destination", 1920, 1080, ImageFormat::RGB32F()));
    m_destination->texture(0)->visualization.documentGamma = 2.1f;

    // Load the environment maps
    Texture::Specification evtSpec;
    evtSpec.assumeSRGBSpaceForAuto = false;
    evtSpec.dimension = Texture::DIM_CUBE_MAP;

    //evtSpec.filename = FilePath::concat(System::findDataFile("cubemap/whiteroom"), "whiteroom-*.png");
    //evtSpec.encoding = Texture::Encoding(ImageFormat::RGB32F(), FrameName::NONE, 1.0f);
    //environmentMapArray.append(Texture::create(evtSpec));

    evtSpec.filename = FilePath::concat(System::findDataFile("cubemap/whiteroom"), "whiteroom-*.png");
    evtSpec.encoding = Texture::Encoding(ImageFormat::RGB32F(), FrameName::NONE, 0.5f);
    environmentMapArray.append(Texture::create(evtSpec));

    evtSpec.filename = FilePath::concat(System::findDataFile("cubemap/plainsky"), "null_plainsky512_*.jpg");
    evtSpec.encoding = Texture::Encoding(ImageFormat::RGB32F(), FrameName::NONE, 1.0f);
    environmentMapArray.append(Texture::create(evtSpec));

    //evtSpec.filename = FilePath::concat(System::findDataFile("cubemap/noonclouds"), "noonclouds_*.png");
    //evtSpec.encoding = Texture::Encoding(ImageFormat::RGB32F(), FrameName::NONE, 1.0f);
    //environmentMapArray.append(Texture::create(evtSpec));

    //evtSpec.filename = FilePath::concat(System::findDataFile("cubemap/doge2"), "doge2-*.exr");
    //evtSpec.encoding = Texture::Encoding(ImageFormat::RGB32F(), FrameName::NONE, 11.0f);
    //environmentMapArray.append(Texture::create(evtSpec));
    
    //evtSpec.filename = FilePath::concat(System::findDataFile("cubemap/ennis"), "ennis-*.exr");
    //evtSpec.encoding = Texture::Encoding(ImageFormat::RGB32F(), FrameName::NONE, 11.0f);
    //environmentMapArray.append(Texture::create(evtSpec));

    // Removed this environment map
    //evtSpec.filename = FilePath::concat(System::findDataFile("cubemap/pisa"), "pisa-*.exr");
    //evtSpec.encoding = Texture::Encoding(ImageFormat::RGB32F(), FrameName::NONE, 8.0f);
    //environmentMapArray.append(Texture::create(evtSpec));

#   if BATCH_PROCESS

        Array<String> categoryArray;
        if (SOURCE_DEPTH == 2) {
            FileSystem::getDirectories(FilePath::concat(SOURCE_PATH, "*"), categoryArray, false);
        } else {
            categoryArray.append(".");
        }

        for (const String& category : categoryArray) {
            // Process the directory
            Array<String> materialDirectoryArray;
            FileSystem::getDirectories(FilePath::concat(SOURCE_PATH, FilePath::concat(category, "*")), materialDirectoryArray, false);
            for (const String& material : materialDirectoryArray) {
                const String& directory = category != "." ? FilePath::concat(category, material) : material;
                debugPrintf("%s\n", directory.c_str());
                processOneMaterial(directory);
            }
        }

        ::exit(0);
#   endif

    // For debugging, allow the program to continue to interactively view the results
    source = Source(
    //    "D:/metamers/categories/brick/castle_brick_01_2k_png"
        "input/Bricks005/2k"
    );
}


void App::processOneMaterial(const String& directory) const {
    const Radiance3 lightRadiance = Radiance3(1, 0.95f, 0.8f) * 3.0f;

    Source source(FilePath::concat(SOURCE_PATH, directory));

    // The size of the output is number_of_environment_maps * NUM_LIGHT_DIRECTIONS
    // See `onInit()` for the environment map list
    const int NUM_LIGHT_DIRECTIONS = 2;

    for (int E = 0; E < environmentMapArray.length(); ++E) {
        for (int L = 0; L < NUM_LIGHT_DIRECTIONS; ++L) {
            const float lightChoice = float(L) / float(NUM_LIGHT_DIRECTIONS - 1);
            const Vector3 directionToLight = Vector3(lerp(-2, -0.1f, lightChoice), lerp(0.0f, 0.6f, lightChoice), 0.6f).direction();
            render(source, directionToLight, lightRadiance, environmentMapArray[E], m_destination);

            const shared_ptr<Image>& image = m_destination->texture(0)->toImage(ImageFormat::RGB8());
            const String& destFilename = FilePath::concat(FilePath::concat(DEST_PATH, directory), format("baked-%03d", E * NUM_LIGHT_DIRECTIONS + L) + ".png");
            image->save(destFilename);
            //debugPrintf("  -> %s\n", destFilename.c_str());
        }
    }
}


void App::makeGUI() {
    developerWindow->cameraControlWindow->setVisible(false);
    developerWindow->sceneEditorWindow->setVisible(false);
    developerWindow->setVisible(false);
    showRenderingStats = false;
}


void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    // There is no 3D in this program, so just override to draw a white background
    rd->setColorClearValue(Color3::white());
    rd->clear();
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    // Render every frame for debugging
    const Radiance3 lightRadiance = Radiance3(1, 0.95f, 0.8f) * 3.0f;

    int environmentMapChoice = int(System::time() * 0.5f) % environmentMapArray.length();
    float lightChoice = float(fabs(cos(System::time())));
    const Vector3 directionToLight = Vector3(lerp(-2, -0.1f, lightChoice), lerp(0.0f, 0.6f, lightChoice), 0.6f).direction();
    render(source, directionToLight, lightRadiance, environmentMapArray[environmentMapChoice], m_destination);

    // Draw the processed image
    Draw::rect2D(rd->viewport().largestCenteredSubRect((float)m_destination->width(), (float)m_destination->height()),
                 rd, Color3::white(), m_destination->texture(0), Sampler::video(), false);
 
    // Render 2D objects such as Widgets.  These do not receive tone mapping or gamma correction.
    Surface2D::sortAndRender(rd, posed2D);
}

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);
    settings.window.caption = "Lighting Bake Example";
    settings.window.width = 1920; settings.window.height = 1080;
    settings.window.fullScreen = false;
    settings.window.resizable = true;
    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(0, 0);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir = FileSystem::currentDirectory();

    return App(settings).run();
}
