# Dev Tools

## G3D Provided
- `log.txt`: records information about startup, the system, major errors, and files accessed. If your program crashes unexpectedly, then this is the first place to look.
- **G3D::DeveloperWindow**: Master control for other GUI debugging tools. Accessed with `f11`.
- **Rendering Statistics bar**: Shows the CPU time spent in the major event handlers and diagnostic information about draw calls, the GPU and build.
- **G3D::CameraControlWindow**: Shows information about the `G3D::GApp::activeCamera` (the current camera) as well as `G3D::GApp::debugCamera`. `F2` toggles between cameras.
  > When the G3D::GApp::debugCamera is active, you can fly it using standard first-person controls (because it runs a G3D::FirstPersonManipulator). These are W, A, S, and D keys for translation, and the mouse for rotation. Because you also need the mouse to click on GUI elements, to actually rotate the camera hold down the right mouse button (or ctrl + left button on a Mac).
- **G3D::VideoRecordDialog**: Records screenshots and videos. `F4` for image, `F6` for video.
  > The GUI presents several options for resolution and format. When an image is captured, you can either save it directly to disk or have the result archived in your development journal (journal.md.html or journal.dox). If saved to the development journal, you can add a caption and additional text. The filename will automatically encode the caption and, if you're using SVN for your project, include the SVN revision number and allow automatically adding the image or video to SVN.
- **G3D::Profiler**: The timer icon in the developer window allows you to record a full profile tree of events on the CPU and GPU. You can manually mark event boundaries in your code using `BEGIN_PROFILER_EVENT` and `END_PROFILER_EVENT`. All `LAUNCH_SHADER` calls are automatically marked.
- **G3D::TextureBrowserWindow**: Accessed via the developer window. Allows you to view any `G3D::Texture` in memory, and create displays on screen for use in screenshots or videos.
- **G3D::SceneEditorWindow**: contains powerful controls for creating and modifying `G3D::Scenes`. It begins in "locked" mode to prevent accidentally changing the scene. The scene editor window provides several features:
  - Loading (and saving) of `.Scene`.Any files
  - Click on objects in the 3D view to select them. Information about selected objects is displayed in the Info pane.
  - A `G3D::ThirdPersonManipulator` displays for the selected object so that it can be moved using the GUI
  - The `G3D::PhysicsFrameSplineEditor` allows creating smooth, scripted motions for (see also the `G3D::Entity::Track` system for even more complex motion)
  - Drag and drop any 3D file or `.ArticulatedModel`.Any from the file system into the 3D view to add it to the scene
  - Drag and drop any `.Scene`.Any file to load it
  - Edit the properties of all `G3D::Entitys` in the scene, including lights and cameras
  - Alter the `G3D::AmbientOcclusion` settings for the scene
  
- **G3D::GApp::debugWindow** and **G3D::GApp::debugPane**:  are initialized by default for easily adding temporary controls to the GUI when debugging. Their visibility can be toggled from the developer window.
  Three functions provide formatted text output for debugging:
  - G3D::debugPrintf writes to stderr on OS X and Linux and to the Visual Studio output window on Windows
  - G3D::screenPrintf writes to the application window. It is useful for output that repeats every frame that you want to watch change. The visibility of the screenPrintf output can be toggled from the developer window
  - G3D::logPrintf writes to log.txt. It is guaranteed to be committed to disk before the function returns. All of the printf variations are slow, but this is really slow because of the disk access

- **G3D::debugDraw**: provides a means to temporarily add geometric primitives to rendering without making them part of the scene. It can display for a single frame (e.g., useful for visualizing values that change every frame) or for a fixed duration (e.g., useful for debugging collisions). 
