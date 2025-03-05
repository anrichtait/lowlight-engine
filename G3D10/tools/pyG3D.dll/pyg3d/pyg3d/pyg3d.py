import os, sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))

major_minor = (sys.version_info.major, sys.version_info.minor)

# Check for supported Python versions
if major_minor < (3, 8) or major_minor > (3, 12):
    raise RuntimeError("This package supports Python 3.8 to 3.12.")

if major_minor == (3, 12):
    import pyG3D_3_12 as g3d
elif major_minor == (3, 11):
    import pyG3D_3_11 as g3d
elif major_minor == (3, 10):
    import pyG3D_3_10 as g3d
elif major_minor == (3, 9):
    import pyG3D_3_9 as g3d
elif major_minor == (3, 8):
    import pyG3D_3_8 as g3d
else:
    print("system version info: ", major_minor)
    raise RuntimeError("Unsupported Python version. Supported versions are 3.8 to 3.12.")

class pyg3d:

    m_scene = None
    m_activeCamera = None

    m_gbufferSpecification = g3d.GBufferSpecification()
    m_hasUserCreatedRenderDevice = False
    m_hasUserCreatedWindow = False
    m_renderDevice = None 
    m_window = None
    m_deviceFramebuffer = None
    m_osWindowDeviceFramebuffer = None
    m_film = None
    m_osWindowHDRFramebuffer = None
    m_framebuffer = None
    m_submitToDisplayMode = None
    m_settings = None
    m_ambientOcclusion = None
    m_gbuffer = None
    m_posed3D = g3d.Array_Surface_SP()
    m_posed2D = g3d.Array_Surface2D_SP()
    m_previousSimTimeStep = None
    m_renderer = None
    m_depthPeelFramebuffer = None
        
    @staticmethod
    def loadScene(sceneName):
        print("Loading scene", str(sceneName), "...")
        if sceneName.endswith('rbxlx'):
            pyg3d.m_scene.loadRBXL(sceneName)
        else:
            pyg3d.m_scene.load(sceneName)
        pyg3d.m_activeCamera = pyg3d.m_scene.defaultCamera()
    
    @staticmethod
    def insertEntity(entity):
        pyg3d.m_scene.insert(entity)
        visibleEntity = g3d.VisibleEntity.create(entity.name(), pyg3d.m_scene, entity)
        pyg3d.m_scene.insert(visibleEntity)
        
    @staticmethod
    def resize(w, h):
        w = max(8, w)
        h = max(8, h)
        hdrSize = pyg3d.m_settings.hdrFramebuffer.hdrFramebufferSizeFromDeviceSize(g3d.Vector2int32(w, h))

        color0 = pyg3d.m_osWindowHDRFramebuffer.texture(0)
        if (not pyg3d.m_film is None) and (not pyg3d.m_window.isIconified()) and ((color0 is None) or (pyg3d.m_osWindowHDRFramebuffer.width() != hdrSize.x) or (pyg3d.m_osWindowHDRFramebuffer.height() != hdrSize.y)):
            pyg3d.m_osWindowHDRFramebuffer.clear()
            stencilBits = g3d.getStencilBits(pyg3d.m_settings)
            pyg3d.m_osWindowHDRFramebuffer.set(g3d.AttachmentPoint.COLOR0, g3d.Texture.getColorTex(pyg3d.m_settings, g3d.String("G3D::GApp::pyg3d.m_osWindowHDRFramebuffer/color"), hdrSize))
            if not g3d.checkImageNone(pyg3d.m_settings):
                p = g3d.AttachmentPoint.DEPTH_AND_STENCIL if stencilBits > 0 else g3d.AttachmentPoint.DEPTH
                depthTexHDR = g3d.Texture.getDepthTex(pyg3d.m_settings, g3d.String("G3D::GApp::pyg3d.m_osWindowHDRFramebuffer/depth"), hdrSize)
                depthTexPeel = g3d.Texture.getDepthTex(pyg3d.m_settings, g3d.String("G3D::GApp::pyg3d.m_depthPeelFramebuffer/depth"), hdrSize)
                pyg3d.m_osWindowHDRFramebuffer.set(p, depthTexHDR)
                pyg3d.m_depthPeelFramebuffer = g3d.Framebuffer.create(depthTexPeel)

    @staticmethod
    def initializeOpenGL(rd, window, createWindowIfNull, settings):
        """
        Initializes EGL window if not already created, sets up rendering and buffers.
        """
        if not rd is None:
            assert(not window is None)
            pyg3d.m_hasUserCreatedRenderDevice = True
            pyg3d.m_hasUserCreatedWindow = True
            pyg3d.m_renderDevice = rd
        elif createWindowIfNull:
            pyg3d.m_hasUserCreatedRenderDevice = False
            pyg3d.m_renderDevice = g3d.RenderDevice.new()
            if not window is None:
                pyg3d.m_renderDevice.init(window)
            else:
                pyg3d.m_hasUserCreatedWindow = False 
                pyg3d.m_renderDevice.init(settings.window)
        
        pyg3d.m_window = pyg3d.m_renderDevice.window()
        g3d.OSWindow.pushGraphicsContext(pyg3d.m_window)
        pyg3d.m_deviceFramebuffer = pyg3d.m_window.framebuffer()
        pyg3d.m_osWindowDeviceFramebuffer = pyg3d.m_window.framebuffer()

        pyg3d.m_film = g3d.Film.create()
        assert(pyg3d.m_renderDevice.width() == settings.window.width) 
        pyg3d.m_osWindowHDRFramebuffer = g3d.Framebuffer.create("GApp::pyg3d.m_osWindowHDRFramebuffer") 
        pyg3d.m_framebuffer = pyg3d.m_osWindowHDRFramebuffer

        pyg3d.resize(settings.window.width, settings.window.height)

        pyg3d.m_renderDevice.setColorClearValue(g3d.Color3(.1, .5, 1))

        pyg3d.m_gbufferSpecification.init() 

        pyg3d.m_renderer = g3d.RendererSettings.factory(settings.renderer) if (not g3d.RendererSettings.hasFactory(settings.renderer)) else g3d.DefaultRenderer.create()
        defaultRenderer = g3d.DefaultRenderer.fromRenderer(pyg3d.m_renderer)

        if (settings.renderer.deferredShading and not defaultRenderer is None):
            g3d.GBufferSpecification.deferredShading(pyg3d.m_gbufferSpecification) 

            defaultRenderer.setDeferredShading(True)

        if defaultRenderer is not None:
            defaultRenderer.setOrderIndependentTransparency(settings.renderer.orderIndependentTransparency)

        hdrSize = pyg3d.m_settings.hdrFramebuffer.hdrFramebufferSizeFromDeviceSize(g3d.Vector2int32(pyg3d.m_deviceFramebuffer.vector2Bounds()))
        pyg3d.m_gbuffer = g3d.GBuffer.create(pyg3d.m_gbufferSpecification) 
        pyg3d.m_osWindowGBuffer = pyg3d.m_gbuffer
        pyg3d.m_gbuffer.resize(hdrSize)
        assert pyg3d.m_osWindowHDRFramebuffer.width() == pyg3d.m_gbuffer.width()
        pyg3d.m_osWindowHDRFramebuffer.set(g3d.AttachmentPoint.DEPTH, g3d.GBuffer.texture(pyg3d.m_gbuffer, g3d.GBufferField.DEPTH_AND_STENCIL))
        pyg3d.m_ambientOcclusion = g3d.AmbientOcclusion.create()

        pyg3d.m_renderDevice.setSwapBuffersAutomatically(False)

    @staticmethod
    def onGraphics3D(rd, allSurfaces):
        if pyg3d.m_scene is None:
            # Swapping buffers updates frame. 
            if pyg3d.m_submitToDisplayMode == g3d.SubmitToDisplayMode.MAXIMIZE_THROUGHPUT and not rd.swapBuffersAutomatically():
                pyg3d.m_renderDevice.swapBuffers()
            rd.clear()
            rd.pushState()
            rd.setProjectionAndCameraMatrix(pyg3d.m_activeCamera.projection(), pyg3d.m_activeCamera.frame())
            rd.popState()
            return
      
        framebufferSize = pyg3d.m_settings.hdrFramebuffer.hdrFramebufferSizeFromDeviceSize(g3d.Vector2int32(pyg3d.m_deviceFramebuffer.vector2Bounds()))
        pyg3d.m_framebuffer.resize(framebufferSize)
        pyg3d.m_gbuffer.resize(framebufferSize)
        pyg3d.m_gbuffer.prepare(rd, pyg3d.m_activeCamera, 0, -pyg3d.m_previousSimTimeStep, pyg3d.m_settings.hdrFramebuffer.depthGuardBandThickness, pyg3d.m_settings.hdrFramebuffer.colorGuardBandThickness)
        dpf = pyg3d.m_depthPeelFramebuffer if pyg3d.m_scene.lightingEnvironment().ambientOcclusionSettings.enabled else None
        pyg3d.m_renderer.render(rd, pyg3d.m_activeCamera, pyg3d.m_framebuffer, dpf, pyg3d.m_scene.lightingEnvironment(), pyg3d.m_gbuffer, allSurfaces, pyg3d.m_scene)
        rd.pushState(pyg3d.m_framebuffer)
        rd.setProjectionAndCameraMatrix(pyg3d.m_activeCamera.projection(), pyg3d.m_activeCamera.frame())
        pyg3d.m_scene.visualize(rd, None, allSurfaces, g3d.SceneVisualizationSettings(), pyg3d.m_activeCamera)

        rd.popState()

        # Swapping buffers updates frame. By default, buffers are swapped here.
        if pyg3d.m_submitToDisplayMode == g3d.SubmitToDisplayMode.MAXIMIZE_THROUGHPUT:
            pyg3d.m_renderDevice.swapBuffers()

        rd.clear()
        pyg3d.m_film.exposeAndRender(rd, pyg3d.m_activeCamera.filmSettings(), pyg3d.m_framebuffer.texture(0), pyg3d.m_settings.hdrFramebuffer.trimBandThickness().x, pyg3d.m_settings.hdrFramebuffer.depthGuardBandThickness.x, g3d.Texture.opaqueBlackIfNull(pyg3d.m_gbuffer.texture(g3d.GBufferField.SS_POSITION_CHANGE) if not pyg3d.m_gbuffer is None else None), pyg3d.m_activeCamera.jitterMotion())                                                         

    @staticmethod
    def onGraphics(rd, posed3D, posed2D):
        rd.pushState()
        assert(not pyg3d.m_activeCamera is None)
        rd.setProjectionAndCameraMatrix(pyg3d.m_activeCamera.projection(), pyg3d.m_activeCamera.frame())
        pyg3d.onGraphics3D(rd, posed3D)
        rd.popState()

    @staticmethod
    def onPose(surface, surface2D):
        if pyg3d.m_scene:
            pyg3d.m_scene.onPose(surface)

    @staticmethod
    def render():
        """
        Effectively runs GApp oneFrame but only with onGraphics and onPose
        """
        pyg3d.m_posed3D.fastClear()
        pyg3d.m_posed2D.fastClear()
        pyg3d.onPose(pyg3d.m_posed3D, pyg3d.m_posed2D)
        pyg3d.m_activeCamera.onPose(pyg3d.m_posed3D)

        if pyg3d.m_submitToDisplayMode == g3d.SubmitToDisplayMode.BALANCE and not pyg3d.m_renderDevice.swapBuffersAutomatically():
            pyg3d.m_renderDevice.swapBuffers()
        
        pyg3d.m_renderDevice.beginFrame()
        pyg3d.m_renderDevice.pushState()
        pyg3d.onGraphics(pyg3d.m_renderDevice, pyg3d.m_posed3D, pyg3d.m_posed2D)
        pyg3d.m_renderDevice.popState()
        pyg3d.m_renderDevice.endFrame()
        if pyg3d.m_submitToDisplayMode == g3d.SubmitToDisplayMode.MINIMIZE_LATENCY and not pyg3d.m_renderDevice.swapBuffersAutomatically():
            pyg3d.m_renderDevice.swapBuffers()
            
        pyg3d.m_posed3D.fastClear()

    def screenshot(filename):
        screenshot = pyg3d.m_renderDevice.screenshotPic()
        screenshot.save(filename) 
        print("screenshot saved as", filename)

    @staticmethod
    def initScene():
        "Creates an empty scene"
        pyg3d.m_scene = g3d.Scene.create(pyg3d.m_ambientOcclusion)

    
    @staticmethod
    def init():
        settings = g3d.GAppSettings()
        settings.writeLicenseFile = False
        settings.window.api = g3d.OSWindowSettingsAPI.API_OPENGL_EGL
        settings.useDeveloperTools = False

        settings.window.width = 512
        settings.window.height = 512

        pyg3d.initFields(settings, pyg3d.m_window, pyg3d.m_renderDevice, True)
        pyg3d.initScene()

    @staticmethod
    def initFields(settings, window, rd, createWindowIfNull):
        """
        Sets up GApp fields and initializes window + openGL
        """
        
        pyg3d.m_submitToDisplayMode = g3d.SubmitToDisplayMode.MAXIMIZE_THROUGHPUT
        pyg3d.m_settings = settings
        assert(not pyg3d.m_settings is None)
        pyg3d.m_previousSimTimeStep = 1/60

        if createWindowIfNull or not window is None:
            pyg3d.initializeOpenGL(pyg3d.m_renderDevice, window, createWindowIfNull, settings)
    
    @staticmethod
    def cleanup():
        pyg3d.m_scene.clear()
        pyg3d.m_posed3D.clear()
        pyg3d.m_posed2D.clear()
        g3d.NetworkDevice.cleanup()
        g3d.VertexBuffer.cleanupAllVertexBuffers()
        if not pyg3d.m_hasUserCreatedRenderDevice and pyg3d.m_hasUserCreatedWindow:
            pyg3d.m_renderDevice.cleanup()


        g3d.GLPixelTransferBuffer.deleteAllBuffers()
        g3d.System.cleanup()
        
        # reset all fields
        pyg3d.m_scene = None
        pyg3d.m_activeCamera = None

        pyg3d.m_gbufferSpecification = g3d.GBufferSpecification()
        pyg3d.m_hasUserCreatedRenderDevice = False
        pyg3d.m_hasUserCreatedWindow = False
        pyg3d.m_deviceFramebuffer = None
        pyg3d.m_osWindowDeviceFramebuffer = None
        pyg3d.m_film = None
        pyg3d.m_osWindowHDRFramebuffer = None
        pyg3d.m_framebuffer = None
        pyg3d.m_submitToDisplayMode = None
        pyg3d.m_settings = None
        pyg3d.m_ambientOcclusion = None
        pyg3d.m_gbuffer = None
        pyg3d.m_posed3D = g3d.Array_Surface_SP()
        pyg3d.m_posed2D = g3d.Array_Surface2D_SP()
        pyg3d.m_previousSimTimeStep = None
        pyg3d.m_renderer = None
        pyg3d.m_depthPeelFramebuffer = None

