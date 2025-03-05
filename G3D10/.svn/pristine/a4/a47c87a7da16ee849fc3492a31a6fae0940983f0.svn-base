/**
  \file G3D-app.lib/include/G3D-app/VideoRecordDialog.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_VideoRecordDialog_h
#define G3D_VideoRecordDialog_h

#include "G3D-base/platform.h"
#include "G3D-app/Widget.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiCheckBox.h"
#include "G3D-app/GuiDropDownList.h"
#include "G3D-app/GuiButton.h"
#include "G3D-app/GuiNumberBox.h"
#include "G3D-app/VideoOutput.h"

namespace G3D {

class RenderDevice;


/**
   @brief A widget that allows the user to launch recording of the
   on-screen image to a movie.

   The playback rate is the frames-per-second value to be stored in
   the movie file.  The record rate <code>1 / G3D::GApp::simTimeStep</code>.

   Set enabled to false to prevent hot-key handling.
 */
class VideoRecordDialog : public GuiWindow {
public:
    friend class GApp;
    friend class Texture;
protected:
    GApp*                        m_app;

    /** For drawing messages on the screen */
    shared_ptr<GFont>            m_font;

    Array<VideoOutput::Encoder>  m_encoders;

    /** Parallel array to m_settingsTemplate of the descriptions for
        use with a drop-down list. */
    Array<String>                m_encoderNames;

    /** Index into m_encoders and m_formatList */
    int                          m_encoderIndex;

    Array<String>                m_ssFormatList;

    /** Index into m_ssFormatList */
    int                          m_ssFormatIndex;

    float                        m_playbackFPS;
    float                        m_recordFPS;

    bool                         m_halfSize;
    bool                         m_enableMotionBlur;
    int                          m_motionBlurFrames;

    /** Recording modifies the GApp::simTimeStep; this is the old value */
    SimTime                      m_oldSimTimeStep;
    RealTime                     m_oldRealTimeTargetDuration;

    float                        m_quality;

    /** For downsampling */
    shared_ptr<Texture>          m_downsampleSrc;
    shared_ptr<Texture>          m_downsampleDst;
    shared_ptr<Framebuffer>      m_downsampleFBO;
    
    /** Motion blur frames */
    GuiNumberBox<int>*           m_framesBox;

    bool                         m_captureGUI;

    /** Draw a software cursor on the frame after capture, since the
     hardware cursor will not be visible.*/
    bool                         m_showCursor;

    GuiButton*                   m_recordButton;

    /** Key to start/stop recording even when the GUI is not
        visible.
      */
    GKey                         m_hotKey;
    GKeyMod                      m_hotKeyMod;

    /** Hotkey + mod as a human readable string */
    String                       m_hotKeyString;
    
    // Screenshot keys
    GKey                         m_ssHotKey;
    GKeyMod                      m_ssHotKeyMod;
    String                       m_ssHotKeyString;

    /** May include a directory */
    String                       m_filenamePrefix;

    VideoRecordDialog(const shared_ptr<GuiTheme>& theme, GApp* app);

    /** Called from constructor */
    void makeGUI();
    
public:

    /** Returns true if the format is supported.  e.g., PNG, JPG, BMP */
    bool setScreenShotFormat(const String& fmt) {
        int i = m_ssFormatList.findIndex(fmt);
        if (i > -1) {
            m_ssFormatIndex = i;
            return true;
        } else {
            return false;
        }
    }

    String screenShotFormat() {
        return m_ssFormatList[m_ssFormatIndex];
    }

    /**
       \param app If not nullptr, the VideoRecordDialog will set the app's
       simTimeStep.

       \param prefix Prefix, which may include a path, of where to store screenshots.
     */
    static shared_ptr<VideoRecordDialog> create(const shared_ptr<GuiTheme>& theme, GApp* app = nullptr);
    static shared_ptr<VideoRecordDialog> create(GApp* app);

    /** Automatically invoked when the record button or hotkey is pressed. 
        Can be called explicitly to force recording.*/
    void startRecording();
    void stopRecording();

    /**
       When false, the screen is captured at the beginning of 
       Posed2DModel rendering from the back buffer, which may 
       slow down rendering.
       
       When true, the screen is captured from the the previous 
       frame, which will not introduce latency into rendering.
    */    
    bool captureGui() const {
        return m_captureGUI;
    }

    /** \copydoc captureGui() */
    void setCaptureGui(bool b) {
        m_captureGUI = b;
    }

    float quality() const {
        return m_quality;
    }

    /** Scales the default bit rate */
    void setQuality(float f) {
        m_quality = f;
    }

    /** Programmatically set the video recording to half size
        (defaults to true).  This can also be changed through the
        GUI. */
    void setHalfSize(bool b) {
        m_halfSize = b;
    }

    bool halfSize() const {
        return m_halfSize;
    }

    virtual void onAI();

    virtual bool onEvent(const GEvent& event);
};

}

#endif
