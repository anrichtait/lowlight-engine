/**
  \file G3D-app.lib/source/ScreenCapture.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/FileSystem.h"
#include "G3D-base/fileutils.h"
#include "G3D-base/Journal.h"
#include "G3D-base/System.h"
#include "G3D-base/TextInput.h"
#include "G3D-app/Draw.h"
#include "G3D-app/FileDialog.h"
#include "G3D-app/GApp.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/GuiTabPane.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/ScreenCapture.h"

#include <stdio.h>

namespace G3D {

class SaveCaptureDialog : public GuiWindow {
protected:
    friend class ScreenCapture;

    GuiTextureBox*      m_textureBox;
    GuiButton*          m_okButton;
    GuiButton*          m_cancelButton;

    OSWindow*           m_osWindow;

    enum {JOURNAL_TAB, FILE_ONLY_TAB};
    int                 m_currentTab;
    
    enum Location {APPEND, NEW};    
    Location            m_location;
    
    String              m_filename;
    String              m_newEntryName;
    String              m_caption;
    String              m_description;
    bool                m_addToSourceControl;
    bool                m_saved;
    
    SaveCaptureDialog(OSWindow* osWindow, const shared_ptr<GuiTheme>& theme, const String& windowTitle,
        const String& existingEntry, const String& filename, const String& newEntry, bool showJournal, bool enableSourceControl, const String& caption);

public:
    static shared_ptr<SaveCaptureDialog> create(OSWindow* osWindow, const shared_ptr<GuiTheme>& theme, const String& windowTitle,
        const String& filename, const String& existingEntry, const String& newEntry, bool showJournal, bool enableSourceControl, const String& caption) {
        return createShared<SaveCaptureDialog>(osWindow, theme, windowTitle, filename, existingEntry, newEntry, showJournal, enableSourceControl, caption);
    }

    virtual bool onEvent(const GEvent& e);
};


String ScreenCapture::s_appScmRevision;
bool   ScreenCapture::s_appScmIsSvn;

ScreenCapture::ScreenCapture(const shared_ptr<GuiTheme>& theme, GApp* app)
    : m_theme(theme)
    , m_app(app)
    , m_mode(MODE_IDLE)
    , m_captureUI(true)
    , m_skipDialog(false) {

    // Use specified output directory or current directory if not set
    if (! m_app->settings().screenCapture.outputDirectory.empty()) {
        m_outputDirectory = m_app->settings().screenCapture.outputDirectory;

        debugAssertM(FileSystem::exists(m_outputDirectory),
            "GApp::Settings::ScreenCapture.outputDirectory set to non-existent directory " + m_outputDirectory);
    } else {
        m_outputDirectory = FileSystem::currentDirectory();
    }

    // Use specified filename prefix or app name is not set
    if (! m_app->settings().screenCapture.filenamePrefix.empty()) {
        m_filenamePrefix = FilePath::makeLegalFilename(m_app->settings().screenCapture.filenamePrefix);
    }
    else {
        m_filenamePrefix = FilePath::makeLegalFilename(System::appName() + "_");
    }

    // Attempt to find journal based on outputDirectory (could end up finding in different directory)
    m_journalPath = Journal::findJournalFile(m_outputDirectory);
}


void ScreenCapture::takeScreenshot(const String& type, bool captureUI, bool skipFilenameDialog, const String& overrideSavePath) {
    // Can't take screenshots while capturing
    if (m_mode != MODE_IDLE) {
        return;
    }

    m_mode = MODE_SCREENSHOT;
    m_captureUI = captureUI;
    m_skipDialog = skipFilenameDialog;
    if (! overrideSavePath.empty()) {
        m_nextOutputPath = overrideSavePath;
    } else {
        m_nextOutputPath = getNextFilenameBase() + "." + toLower(type);
    }
}


void ScreenCapture::startVideoRecording(const VideoOutput::Settings& settings, bool captureUI, bool skipFilenameDialog, const String& overrideSavePath) {
    // Can't take video while capturing
    if (m_mode != MODE_IDLE) {
        return;
    }

    m_mode = MODE_VIDEO;
    m_captureUI = captureUI;
    m_skipDialog = skipFilenameDialog;
    if (! overrideSavePath.empty()) {
        m_nextOutputPath = overrideSavePath;
    } else {
        m_nextOutputPath = getNextFilenameBase() + settings.encoder.extension;
    }
    m_video = VideoOutput::create(m_nextOutputPath, settings);

    if (isNull(m_video)) {
        m_mode = MODE_IDLE;
    }
}


void ScreenCapture::endVideoRecording() {
    if (m_mode != MODE_VIDEO) {
        return;
    }

    // finish video recording
    m_video->commit();
    m_video.reset();

    saveCapture(m_app->window(), "Save Video");
    m_mode = MODE_IDLE;
}

bool ScreenCapture::isVideoRecording() {
    return m_video.get() != nullptr;
}


String ScreenCapture::getNextFilenameBase() {
    // Generate unique filename (with path)
    String prefix = FilePath::concat(m_outputDirectory, m_filenamePrefix);
    prefix = generateFileNameBaseAnySuffix(prefix);

    String path = prefix;
    if (m_app->settings().screenCapture.includeG3DRevision) {
        path += "_g3drev_" + System::g3dRevision();
    }

    if (m_app->settings().screenCapture.includeAppRevision && !s_appScmRevision.empty()) {
        path += "_apprev_" + s_appScmRevision;
    }

    return path;
}


String ScreenCapture::saveCaptureAs
   (const String&                   existingPath,
    const String&                   windowTitle,
    const shared_ptr<Texture>&      preview,
    bool                            flipPreview,
    const String&                   caption) {
    m_skipDialog = false;
    m_nextOutputPath = existingPath;
    return saveCapture(m_app->window(), windowTitle, preview, flipPreview, caption);
}


void ScreenCapture::saveImageGridToJournal(const String& sectionTitle, const Array<shared_ptr<Texture>>& textureArray, int numColumns, bool addToSCM) {
    int n = 0;
    String markdeep;
    for (const shared_ptr<Texture>& texture : textureArray) {
        const shared_ptr<Image>& result = texture->toImage(ImageFormat::RGB8());
        const String& filename = getNextFilenameBase() + "_" + FilePath::makeLegalFilename(texture->caption()) + ".png";
        result->save(filename);

        if (addToSCM) {
            addCaptureToScm(filename);
        }

        markdeep += format("![%s](%s) ", texture->caption().c_str(), filename.c_str());
        ++n;
        if (n >= numColumns) {
            markdeep += "\n";
            n = 0;
        }
    }
    Journal::insertNewSection(Journal::findJournalFile(), System::currentDateString() + ": Results", markdeep);
}


void ScreenCapture::onAfterGraphics3D(RenderDevice* rd) {
    if (m_captureUI) {
        return;
    }
    processFrame(rd);
}


void ScreenCapture::onAfterGraphics2D(RenderDevice* rd) {
    if (! m_captureUI) {
        return;
    }
    processFrame(rd);
}

void ScreenCapture::processFrame(RenderDevice* rd) {
    switch (m_mode) {
    case MODE_SCREENSHOT:
        takeScreenshot(rd);
        m_mode = MODE_IDLE;
        break;
        
    case MODE_VIDEO:
        recordFrame(rd);
        break;

    default:
        break;
    }
}

void ScreenCapture::takeScreenshot(RenderDevice* rd) {
    const shared_ptr<Image>& screenshot = rd->screenshotPic();
    const shared_ptr<Texture>& preview = Texture::fromImage("Capture Dialog", screenshot, ImageFormat::AUTO(), Texture::DIM_2D, false);
    screenshot->save(m_nextOutputPath);

    saveCapture(m_app->window(), "Save Screenshot", preview);
    m_mode = MODE_IDLE;
}


void ScreenCapture::recordFrame(RenderDevice* rd) {
    if (m_video->settings().width != rd->width()) {
        // Downsample if video is recording different size than RenderDevice
        if (isNull(m_downsampleSrc)) {
            const bool generateMipMaps = false;
            m_downsampleSrc = Texture::createEmpty("Downsample Source", 16, 16,
                                                   TextureFormat::RGB8(), Texture::DIM_2D, generateMipMaps);
        }

        // Grab full frame
        RenderDevice::ReadBuffer old = rd->readBuffer();
        rd->setReadBuffer(RenderDevice::READ_BACK);
        rd->copyTextureFromScreen(m_downsampleSrc, Rect2D::xywh(0,0,float(rd->width()), float(rd->height())));
        rd->setReadBuffer(old);

        if (isNull(m_downsampleFBO)) {
            m_downsampleFBO = Framebuffer::create("Downsample Framebuffer");
        }

        if (isNull(m_downsampleDst) || 
            (m_downsampleDst->width() != m_downsampleSrc->width() / 2) || 
            (m_downsampleDst->height() != m_downsampleSrc->height() / 2)) {
            // Allocate destination
            const bool generateMipMaps = false;
            m_downsampleDst = 
                Texture::createEmpty("Downsample Destination", m_video->settings().width, 
                                     m_video->settings().height, 
                                     ImageFormat::RGB8(), Texture::DIM_2D, generateMipMaps);
            m_downsampleFBO->set(Framebuffer::COLOR0, m_downsampleDst);
        }

        // Downsample (use bilinear for fast filtering
        const bool wasFrameInverted = rd->invertY();
        rd->push2D(m_downsampleFBO);
        {
            const Vector2& halfPixelOffset = Vector2(0.5f, 0.5f) / m_downsampleDst->vector2Bounds();
            Draw::rect2D(m_downsampleDst->rect2DBounds() + halfPixelOffset, rd, Color3::white(), m_downsampleSrc, Sampler::video(), wasFrameInverted);
        }
        rd->pop2D();

        // Write downsampled texture to the video
        m_video->append(m_downsampleDst);

    } else {
        // Full-size: grab directly from the screen
        m_video->append(rd, true);
    }
}


String ScreenCapture::saveCapture(OSWindow* osWindow, const String& windowTitle, shared_ptr<Texture> preview, bool flipPreview, const String& caption) {
    if (m_skipDialog) {
        return m_nextOutputPath;
    }

    String lastEntryName;
    if (! m_journalPath.empty()) {
        lastEntryName = Journal::firstSectionTitle(m_journalPath);

        // Clear entry name if new date to force new entry
        if (! beginsWith(lastEntryName, System::currentDateString())) {
            lastEntryName.clear();
        }
    }

    const shared_ptr<SaveCaptureDialog>& dialog =
        SaveCaptureDialog::create(osWindow, m_theme, windowTitle, m_nextOutputPath, lastEntryName,
            "New Section", ! m_journalPath.empty(), m_app->settings().screenCapture.addFilesToSourceControl && !s_appScmRevision.empty(), caption);

    dialog->m_textureBox->setTexture(preview, flipPreview);
    dialog->showModal(osWindow);

    if (! dialog->m_saved) {
        FileSystem::removeFile(m_nextOutputPath);
        return "";
    }

    if (dialog->m_currentTab == SaveCaptureDialog::JOURNAL_TAB) {
        if (! dialog->m_caption.empty()) {
            // Add entry caption to filename to make it easy to find
            const String& path = FilePath::parent(dialog->m_filename);
            const String& ext  = FilePath::ext(dialog->m_filename);
            const String& s    = FilePath::base(dialog->m_filename) + "_" + FilePath::makeLegalFilename(dialog->m_caption);
            dialog->m_filename = FilePath::concat(path, s + "." + ext);
        }

        const String& entryText = Journal::formatImage(m_journalPath, dialog->m_filename, dialog->m_caption, wordWrap(dialog->m_description, 80));

        // Add journal entry
        switch (dialog->m_location) {
        case SaveCaptureDialog::APPEND:
            Journal::appendToFirstSection(m_journalPath, entryText);
            break;

        case SaveCaptureDialog::NEW:
            Journal::insertNewSection(m_journalPath, System::currentDateString() + ": " + dialog->m_newEntryName, entryText);
            break;

        default:
            break;
        }
    }

    if (dialog->m_filename != m_nextOutputPath) {
        // Move to save as location
        FileSystem::rename(m_nextOutputPath, dialog->m_filename);
    }

    if (dialog->m_currentTab == SaveCaptureDialog::JOURNAL_TAB && dialog->m_addToSourceControl) {
        // Add file to soure control if in journal tab and checked
        addCaptureToScm(dialog->m_filename);
    }
    
    return dialog->m_filename;
}


String ScreenCapture::runCommand(const String& command) {
    FILE* pipe = 
#ifdef G3D_WINDOWS
    _popen(command.c_str(), "r");
#else
    popen(command.c_str(), "r");
#endif

    if (pipe == nullptr)
    {
        return String();
    }

    char buffer[128];
    String result;
    while (! feof(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
    }

#ifdef G3D_WINDOWS
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return result;
}


void ScreenCapture::checkAppScmRevision(const String& outputDirectory) {

    std::thread ([outputDirectory]() {

        //current directory is used if outputDirectory is empty
        String version = runCommand("svnversion " + outputDirectory);

        if (version.empty() || beginsWith(version, "Unversioned directory")) {
            //Only need latest revision
            version = runCommand("git log -1 " + outputDirectory);
        } else {
            const size_t separatorIndex = version.find(':');
            if (separatorIndex != String::npos) {
                version = version.substr(separatorIndex + 1);
            }

            TextInput ti(TextInput::FROM_STRING, version);
            const Token& t = ti.read();
            if (t.extendedType() == Token::INTEGER_TYPE) {
                s_appScmRevision = format("%d", (int)t.number());
            }
            s_appScmIsSvn = true;
            return;
        }

        // If we're here and version is still empty, neither git nor svn versions the output directory
        if (version.empty() || beginsWith(version, "fatal: Not a git repository")) {
            // Leave s_appScmRevision empty
            return;
        }
        
        s_appScmIsSvn = false;

        //Commit hash
        size_t startIndex = version.find(" ", version.find("commit ")) + 1;
        //(HEAD -> temp) is not included in command output.
        size_t endIndex = version.find("\n", startIndex); 
        if (startIndex != String::npos && endIndex != String::npos) {
            s_appScmRevision = version.substr(startIndex, endIndex - startIndex);
        }


        //Author
        startIndex = version.find("<") + 1;
        endIndex = version.find("@", startIndex);
        if ((startIndex != String::npos) && (endIndex != String::npos)) {
            s_appScmRevision += version.substr(startIndex, endIndex - startIndex);
        }


    }).detach();

}


void ScreenCapture::addCaptureToScm(const String& path) {
    s_appScmIsSvn ? runCommand("svn add " + path) : runCommand("git add " + path);
}


SaveCaptureDialog::SaveCaptureDialog
   (OSWindow*                       osWindow, 
    const shared_ptr<GuiTheme>&     theme, 
    const String&                   windowTitle, 
    const String&                   filename, 
    const String&                   existingEntry, 
    const String&                   newEntry, 
    bool                            showJournal, 
    bool                            enableSourceControl,
    const String&                   caption)
    : GuiWindow(windowTitle, theme, Rect2D::xywh(400, 100, 10, 10), GuiTheme::DIALOG_WINDOW_STYLE, HIDE_ON_CLOSE)
    , m_osWindow(osWindow)
    , m_location(APPEND)
    , m_filename(filename)
    , m_caption(caption)
    , m_addToSourceControl(true)
    , m_saved(false)
    {

    GuiPane* rootPane = pane();

    GuiTabPane* tabPane = pane()->addTabPane(&m_currentTab);
    GuiPane* journalPane = tabPane->addTab("Journal");
    GuiPane* filePane = tabPane->addTab("File Only");

    GuiRadioButton* appendButton = journalPane->addRadioButton("Append to `" + existingEntry + "'", APPEND, &m_location);
    if (existingEntry.empty()) {
        appendButton->setEnabled(false);
        m_location = NEW;
    }

    journalPane->beginRow();

    m_newEntryName = newEntry;
    GuiRadioButton* newEntryButton = journalPane->addRadioButton("New entry", NEW, &m_location);
    newEntryButton->setWidth(90);
    GuiTextBox* newEntryNameBox = journalPane->addTextBox(GuiText(), &m_newEntryName);
    newEntryNameBox->setWidth(400);
    journalPane->endRow();

    GuiTextBox* captionBox = journalPane->addTextBox("Caption", &m_caption);
    captionBox->setCaptionWidth(90);
    captionBox->moveBy(0, 15);
    captionBox->setWidth(490);

    journalPane->beginRow(); {
        GuiMultiLineTextBox* descriptionBox = journalPane->addMultiLineTextBox("Description", &m_description);
        descriptionBox->setCaptionWidth(90);
        descriptionBox->moveBy(0, 15);
        descriptionBox->setSize(490, 100);
        descriptionBox->setWidth(490);
    } journalPane->endRow();

    journalPane->beginRow(); {
        String sourceControlDesc;
        if (! enableSourceControl) {
            sourceControlDesc = " (outputDirectory must be in svn or git repository.)";
            m_addToSourceControl = false;
        }
        GuiCheckBox* svnCheckbox = journalPane->addCheckBox("Add to Source Control" + sourceControlDesc, &m_addToSourceControl);
        svnCheckbox->setEnabled(enableSourceControl);
    } journalPane->endRow();

    journalPane->pack();

    GuiTextBox* textBox = filePane->addTextBox("Filename", &m_filename, GuiTextBox::IMMEDIATE_UPDATE);
    textBox->setSize(Vector2(min(m_osWindow->width() - 100.0f, 500.0f), textBox->rect().height()));
    textBox->setFocused(true);

    filePane->addButton("...", [this]() {FileDialog::getFilename(m_filename, FilePath::ext(m_filename)); });
    tabPane->pack();

    m_textureBox = rootPane->addTextureBox(nullptr, shared_ptr<Texture>(), true);
    m_textureBox->setSize(256, 256 * min(3.0f, float(osWindow->height()) / osWindow->width()));
    m_textureBox->moveRightOf(tabPane);
    m_textureBox->moveBy(0, 18);

    m_okButton = rootPane->addButton("Ok");
    m_okButton->setPosition(Vector2(tabPane->rect().x1() - m_okButton->rect().width(), m_okButton->rect().y0()));
    m_cancelButton = rootPane->addButton("Cancel");
    m_cancelButton->setPosition(Vector2(m_okButton->rect().x0() - m_cancelButton->rect().width(), m_okButton->rect().y0()));

    if (showJournal) {
        m_currentTab = JOURNAL_TAB;
        captionBox->setFocused(true);
        captionBox->moveCursorEnd();
    } else {
        journalPane->setEnabled(false);
        m_currentTab = FILE_ONLY_TAB;
        m_okButton->setFocused(true);
    }
    
    pack();
    moveToCenter();
}


bool SaveCaptureDialog::onEvent(const GEvent& e) {
    const bool handled = GuiWindow::onEvent(e);

    // Check after the (maybe key) event is processed normally
    m_okButton->setEnabled(!trimWhitespace(m_filename).empty());

    if (handled) {
        return true;
    }

    if ((e.type == GEventType::GUI_ACTION) && 
        ((e.gui.control == m_cancelButton) ||
         (e.gui.control == m_okButton))) {
        m_saved = (e.gui.control == m_okButton);
        
        hideModal();
        return true;

    } else if ((e.type == GEventType::KEY_DOWN) && (e.key.keysym.sym == GKey::ESCAPE)) {
        // Cancel the dialog
        hideModal();
        return true;
    }

    return false;
}


} // namespace G3D
