/**
  \file G3D-app.lib/include/G3D-app/ScreenCapture.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define GLG3D_ScreenCapture_h
#include "G3D-base/G3DString.h"
#include "G3D-base/Rect2D.h"
#include "G3D-app/GuiTheme.h"
#include "G3D-app/VideoOutput.h"

namespace G3D {

class Framebuffer;
class GApp;
class OSWindow;

/**
    \brief Handles screenshot and video capture with managed filename generation and Journal output.

    Allows for easy capture of screenshots and videos to a central directory.  Can be safely used separate
    from the DeveloperWindow GUI to save captures to the same location.  Each capture is given a unique generated filename.
    Allows a single screenshot or video capture at one time.

    Will automatically detect a Journal in the output directory and enable saving Journal entries. \sa Journal

    Will automatically detect if app is in an svn repository (if configured) and allow including app svn revision to filenames
    as well as adding captures to the svn repository via the Journal dialog.
*/
class ScreenCapture {
public:
    ScreenCapture(const shared_ptr<GuiTheme>& theme, GApp* app);

    /** Takes a screenshot and automatically handles saving and prompting the user for Journal and filename confirmation.
        Will modify Journal (if found) and handle source control changes (if selected).  Will remove file if dialog was cancelled.

        \param type The screenshot file format saved and file extension. Can use file format supported by Image. 
        \param captureUI Whether to include the 2D GUI in the screenshot.
        \param skipFilenameDialog Where to automatically accept the generated filename and skip showing Journal and Save As dialog.
        \param overrideSavePath Force screenshot to save to this path.  Can be used with skipFilenameDialog to avoid changing path.
    */
    void takeScreenshot(const String& type = "jpg", bool captureUI = true, bool skipFilenameDialog = false, const String& overrideSavePath = String());

    /** Starts video recording.

        \param type The screenshot file format saved and file extension. Can use file format supported by Image. 
        \param captureUI Whether to include the 2D GUI in the screenshot.
        \param skipFilenameDialog Where to automatically accept the generated filename and skip showing Journal and filename dialog.
        \param overrideSavePath Force screenshot to save to this path.  Can be used with skipFilenameDialog to avoid changing path.
    */
    void startVideoRecording(const VideoOutput::Settings& settings, bool captureUI = true, bool skipFilenameDialog = false, const String& overrideSavePath = String());

    /** Ends video recording and automatically handles saving and prompting the use for Journal and filename confirmation.
        Will modify Journal (if found) and handle source control changes (if selected).  Will remove file if dialog was cancelled.

    */
    void endVideoRecording();
    bool isVideoRecording();

    /** Returns a uniquely generated path in the configured GApp::Settings::ScreenCapture::outputDirectory.  Does not contain extension. */
    String getNextFilenameBase();

    /** Prompts the use for a name to save the image or video as and then renames existing temporary file \a path
        to the one selected by the user.
        
        Will modify Journal (if found) and handle source control changes (if selected). 
        Will delete the file if the dialog is cancelled.
        
        If called too soon (less than 1-2 seconds) after startup, s_appScmIsSvn and s_appScmRevision may not yet be
        initialized, as they are initialized on a detached thread.

        \return Full path to saved file or empty if save was cancelled.
    */
    String saveCaptureAs(const String& path, const String& windowTitle = "Save Capture", const shared_ptr<Texture>& preview = nullptr, bool flipPreview = false, const String& caption = "");

    /** Used internally by GApp to capture frames. */
    void onAfterGraphics3D(RenderDevice* rd);

    /** Used internally by GApp to capture frames. */
    void onAfterGraphics2D(RenderDevice* rd);

    static void checkAppScmRevision(const String& outputDirectory);

    /** Filenames and captions are taken from the \a textureArray */
    void saveImageGridToJournal(const String& sectionTitle, const Array<shared_ptr<Texture>>& textureArray, int numColumns = 3, bool addToSCM = true);

private:

    enum CaptureMode { MODE_IDLE, MODE_SCREENSHOT, MODE_VIDEO };

    void processFrame(RenderDevice* rd);
    void takeScreenshot(RenderDevice* rd);
    void recordFrame(RenderDevice* rd);

    String saveCapture(OSWindow* osWindow, const String& windowTitle, shared_ptr<Texture> preview = nullptr, bool flipPreview = false, const String& caption = "");
    

    static String runCommand(const String& command);
    void   addCaptureToScm(const String& path);

    static String s_appScmRevision;
    static bool   s_appScmIsSvn;

    shared_ptr<GuiTheme>    m_theme;
    GApp*                   m_app;
    String                  m_outputDirectory;
    String                  m_filenamePrefix;
    String                  m_journalPath;

    CaptureMode             m_mode;
    String                  m_nextOutputPath;
    bool                    m_captureUI;
    bool                    m_skipDialog;

    shared_ptr<VideoOutput> m_video;

    /** For downsampling */
    shared_ptr<Texture>     m_downsampleSrc;
    shared_ptr<Texture>     m_downsampleDst;
    shared_ptr<Framebuffer> m_downsampleFBO;
};

} // namespace G3D
