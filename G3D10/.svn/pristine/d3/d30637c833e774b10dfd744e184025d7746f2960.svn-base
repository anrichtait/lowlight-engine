/**
  \file G3D-app.lib/source/VideoRecordDialog.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/Log.h"
#include "G3D-app/Draw.h"
#include "G3D-app/GApp.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/GuiCheckBox.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/VideoRecordDialog.h"
#include "G3D-base/FileSystem.h"
#include "G3D-app/ScreenCapture.h"

namespace G3D {

shared_ptr<VideoRecordDialog> VideoRecordDialog::create(const shared_ptr<GuiTheme>& theme, GApp* app) {
    return shared_ptr<VideoRecordDialog>(new VideoRecordDialog(theme, app));
}


shared_ptr<VideoRecordDialog> VideoRecordDialog::create(GApp* app) {
    alwaysAssertM(app, "GApp may not be nullptr");
    return createShared<VideoRecordDialog>(app->debugWindow->theme(), app);
}


VideoRecordDialog::VideoRecordDialog(const shared_ptr<GuiTheme>& theme, GApp* app) : 
    GuiWindow("Screen Capture", theme, Rect2D::xywh(0, 100, 320, 200),
              GuiTheme::DIALOG_WINDOW_STYLE, GuiWindow::HIDE_ON_CLOSE),
    m_app(app),
    m_encoderIndex(0),
    m_ssFormatIndex(0),
    m_playbackFPS(30),
    m_recordFPS(30),
    m_halfSize(true),
    m_enableMotionBlur(false),
    m_motionBlurFrames(10),
    m_quality(1.0),
    m_framesBox(nullptr),
    m_captureGUI(true),
    m_showCursor(false) {
    m_hotKey = GKey::F6;
    m_hotKeyMod = GKeyMod::NONE;
    m_hotKeyString = m_hotKey.toString();

    m_ssHotKey = GKey::F4;
    m_ssHotKeyMod = GKeyMod::NONE;
    m_ssHotKeyString = m_ssHotKey.toString();

    m_encoders.append(VideoOutput::Encoder::DEFAULT());
    m_encoderNames.append(m_encoders.last().description);
    m_encoders.append(VideoOutput::Encoder::H264_NVIDIA());
    m_encoderNames.append(m_encoders.last().description);
    m_encoders.append(VideoOutput::Encoder::H265_NVIDIA());
    m_encoderNames.append(m_encoders.last().description);
    m_encoders.append(VideoOutput::Encoder::MPEG4());
    m_encoderNames.append(m_encoders.last().description);

    m_font = GFont::fromFile(System::findDataFile("arial.fnt"));

    makeGUI();
}


void VideoRecordDialog::makeGUI() {
    pane()->addCheckBox("Record GUI (Surface2D)", &m_captureGUI);

    pane()->addLabel(GuiText("Video", shared_ptr<GFont>(), 12));
    GuiPane* moviePane = pane()->addPane("", GuiTheme::ORNATE_PANE_STYLE);

    GuiLabel* label = nullptr;
    GuiDropDownList* formatList = moviePane->addDropDownList("Format", m_encoderNames, &m_encoderIndex);

    const float width = 300.0f;
    // Increase caption size to line up with the motion blur box
    const float captionSize = 90.0f;

    formatList->setWidth(width);
    formatList->setCaptionWidth(captionSize);

    moviePane->addNumberBox("Quality", &m_quality, "", GuiTheme::LOG_SLIDER, 0.1f, 25.0f);
    
    if (false) {
        // For future expansion
        GuiCheckBox*  motionCheck = moviePane->addCheckBox("Motion Blur",  &m_enableMotionBlur);
        m_framesBox = moviePane->addNumberBox("", &m_motionBlurFrames, "frames", GuiTheme::LINEAR_SLIDER, 2, 20);
        m_framesBox->setUnitsSize(46);
        m_framesBox->moveRightOf(motionCheck);
        m_framesBox->setWidth(210);
    }

    GuiNumberBox<float>* recordBox   = moviePane->addNumberBox("Record as if",      &m_recordFPS, "fps", GuiTheme::NO_SLIDER, 1.0f, 120.0f, 0.1f);
    recordBox->setCaptionWidth(captionSize);

    GuiNumberBox<float>* playbackBox = moviePane->addNumberBox("Playback at",    &m_playbackFPS, "fps", GuiTheme::NO_SLIDER, 1.0f, 120.0f, 1.0f);
    playbackBox->setCaptionWidth(captionSize);

    const OSWindow* window = OSWindow::current();
    int w = window->width() / 2;
    int h = window->height() / 2;
    moviePane->addCheckBox(format("Half-size (%d x %d)", w, h), &m_halfSize);

    if (false) {
        // For future expansion
        moviePane->addCheckBox("Show cursor", &m_showCursor);
    }

    label = moviePane->addLabel("Hot key:");
    label->setWidth(captionSize);
    moviePane->addLabel(m_hotKeyString)->moveRightOf(label);

    // Add record on the same line as previous hotkey box
    m_recordButton = moviePane->addButton("Record Now (" + m_hotKeyString + ")");
    m_recordButton->moveBy(moviePane->rect().width() - m_recordButton->rect().width() - 5, -27);
    moviePane->pack();
    moviePane->setWidth(pane()->rect().width());

    ///////////////////////////////////////////////////////////////////////////////////
    pane()->addLabel(GuiText("Screenshot", shared_ptr<GFont>(), 12));
    GuiPane* ssPane = pane()->addPane("", GuiTheme::ORNATE_PANE_STYLE);

    m_ssFormatList.append("JPG", "PNG", "BMP", "TGA");
    GuiDropDownList* ssFormatList = ssPane->addDropDownList("Format", m_ssFormatList, &m_ssFormatIndex);
    m_ssFormatIndex = 0;

    ssFormatList->setWidth(width);
    ssFormatList->setCaptionWidth(captionSize);

    label = ssPane->addLabel("Hot key:");
    label->setWidth(captionSize);
    ssPane->addLabel(m_ssHotKeyString)->moveRightOf(label);

    ssPane->pack();
    ssPane->setWidth(pane()->rect().width());

    ///////////////////////////////////////////////////////////////////////////////////

    pack();
    setRect(Rect2D::xywh(rect().x0(), rect().y0(), rect().width() + 5, rect().height() + 2));
}


void VideoRecordDialog::onAI () {
    if (m_framesBox) {
        m_framesBox->setEnabled(m_enableMotionBlur);
    }
}


void VideoRecordDialog::startRecording() {
    // Create the video file
    VideoOutput::Settings settings;
    settings.encoder = m_encoders[m_encoderIndex];

    OSWindow* window = const_cast<OSWindow*>(OSWindow::current());
    settings.width = window->width();
    settings.height = window->height();

    if (m_halfSize) {
        settings.width /= 2;
        settings.height /= 2;
    }

    settings.setBitrateQuality(m_quality);
    settings.fps = (int)m_playbackFPS;

    m_app->screenCapture()->startVideoRecording(settings, m_captureGUI);

    if (m_app->screenCapture()->isVideoRecording()) {
        m_oldSimTimeStep = m_app->simStepDuration();
        m_oldRealTimeTargetDuration = m_app->realTimeTargetDuration();

        m_app->setFrameDuration(1.0f / m_recordFPS, GApp::MATCH_REAL_TIME_TARGET);

        m_recordButton->setCaption("Stop (" + m_hotKeyString + ")");
        setVisible(false);

        // Change the window caption as well
        const String& c = window->caption();
        const String& appendix = " - Recording " + m_hotKeyString + " to stop";
        if (! endsWith(c, appendix)) {
            window->setCaption(c + appendix);
        }
    }
}


void VideoRecordDialog::stopRecording() {
    if (m_app->screenCapture()->isVideoRecording()) {
        m_app->screenCapture()->endVideoRecording();

        // Restore the app state
        m_app->setFrameDuration(m_oldRealTimeTargetDuration, m_oldSimTimeStep);

        // Reset the GUI
        m_recordButton->setCaption("Record Now (" + m_hotKeyString + ")");

        // Restore the window caption as well
        OSWindow* window = const_cast<OSWindow*>(OSWindow::current());
        const String& c = window->caption();
        const String& appendix = " - Recording " + m_hotKeyString + " to stop";
        if (endsWith(c, appendix)) {
            window->setCaption(c.substr(0, c.size() - appendix.size()));
        }
    }
}


bool VideoRecordDialog::onEvent(const GEvent& event) {
    if (GuiWindow::onEvent(event)) {
        // Base class handled the event
        return true;
    }

    if (enabled()) {
        // Video
        const bool buttonClicked = (event.type == GEventType::GUI_ACTION) && (event.gui.control == m_recordButton);
        const bool hotKeyPressed = (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == m_hotKey) && (event.key.keysym.mod == m_hotKeyMod);

        if (buttonClicked || hotKeyPressed) {
            if (m_app->screenCapture()->isVideoRecording()) {
                stopRecording();
            } else {
                startRecording();
            }
            return true;
        }

        const bool ssHotKeyPressed = (event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == m_ssHotKey) && (event.key.keysym.mod == m_ssHotKeyMod);

        if (ssHotKeyPressed) {
            m_app->screenCapture()->takeScreenshot(m_ssFormatList[m_ssFormatIndex], m_captureGUI);
            return true;
        }

    }

    return false;
}

}
