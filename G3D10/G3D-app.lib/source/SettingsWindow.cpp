/**
  \file G3D-app.lib/source/SettingsWindow.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/FileSystem.h"
#include "G3D-app/SettingsWindow.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/GuiTabPane.h"
#include "G3D-gfx/Texture.h"
#include "G3D-app/ArticulatedModel.h"
#include "G3D-app/UniversalMaterial.h"
#include "G3D-base/FileSystem.h"
#include "G3D-gfx/GLSamplerObject.h"
#include "G3D-gfx/GazeTracker.h"
#include "G3D-app/DefaultRenderer.h"
#include "G3D-app/GApp.h"
#include "G3D-gfx/XR.h"
#include "G3D-gfx/RenderDevice.h"

namespace G3D {
SettingsWindow::SettingsWindow
   (const shared_ptr<GuiTheme>& theme,
    GApp*                       app) : 
    GuiWindow("Engine Settings", 
              theme,
              Rect2D::xywh(300, 150, 400, 300),
              GuiTheme::NORMAL_WINDOW_STYLE,
              GuiWindow::HIDE_ON_CLOSE),
    m_app(app) {
   
    GuiPane* root = GuiWindow::pane();

    static int ignore = 0;
    GuiTabPane* tabPane = root->addTabPane(&ignore);
    {
        GuiPane* pane = tabPane->addTab("App");
        pane->setNewChildSize(350, -1, 90);

        static String programBuildType = compiledBuild();
        pane->addTextBox("App Build", &programBuildType)->setEnabled(false);

        static String programCompiled = compiledDate();
        pane->addTextBox("App Compiled", &programCompiled)->setEnabled(false);

        static String g3dBuild = System::version();
        pane->addTextBox("G3D Build", &g3dBuild)->setEnabled(false);

        static String g3dDate = __TIME__ ", " __DATE__;
        pane->addTextBox("G3D Compiled", &g3dDate)->setEnabled(false);

        pane->addTextBox("Renderer", Pointer<String>([this](){ return (isNull(m_app) || isNull(m_app->renderer())) ? "None" : m_app->renderer()->className(); }))->setEnabled(false);
        pane->addTextBox("Shading Mode", Pointer<String>([this](){ if (isNull(m_app)) { return "N/A"; }
            const shared_ptr<DefaultRenderer>& renderer = dynamic_pointer_cast<DefaultRenderer>(m_app->renderer());
            if (isNull(renderer)) { return "N/A"; }
            return renderer->deferredShading() ? "Deferred" : "Forward";
        }))->setEnabled(false);

        pane->addTextBox("Transparency", Pointer<String>([this](){ if (isNull(m_app)) { return "N/A"; }
            const shared_ptr<DefaultRenderer>& renderer = dynamic_pointer_cast<DefaultRenderer>(m_app->renderer());
            if (isNull(renderer)) { return "N/A"; }
            return renderer->orderIndependentTransparency() ? "Order-independent" : "Sorted";
        }))->setEnabled(false);

        pane->addTextBox("Gaze Tracker", Pointer<String>([this](){ return (notNull(m_app) && notNull(m_app->gazeTracker())) ? m_app->gazeTracker()->className() : "None"; }))->setEnabled(false);

        static String& GPU = const_cast<String&>(RenderDevice::current->getCardDescription()); 
        pane->addTextBox("GPU", &GPU)->setEnabled(false);

        pane->addTextBox("XR System", Pointer<String>([this](){ return (notNull(m_app) && notNull(m_app->settings().vr.xrSystem)) ? m_app->settings().vr.xrSystem->className() : "None"; }))->setEnabled(false);
    }
    {
        GuiPane* pane = tabPane->addTab("Caches"); 
        pane->addLabel("Clear:")->setWidth(120);

        pane->addButton("ArticulatedModel",     []() { ArticulatedModel::clearCache(); }, GuiTheme::TOOL_BUTTON_STYLE);
        pane->addButton("UniversalMaterial",    []() { UniversalMaterial::clearCache(); }, GuiTheme::TOOL_BUTTON_STYLE);
        pane->addButton("Texture",              []() { Texture::clearCache(); }, GuiTheme::TOOL_BUTTON_STYLE);
        pane->addButton("FileSystem",           []() { FileSystem::clearCache(); }, GuiTheme::TOOL_BUTTON_STYLE);
        pane->addButton("GLSamplerObject",      []() { GLSamplerObject::clearCache(); }, GuiTheme::TOOL_BUTTON_STYLE);

        pane->addButton("All",                  []() { ArticulatedModel::clearCache();
                                                       UniversalMaterial::clearCache();
                                                       Texture::clearCache();
                                                       FileSystem::clearCache();
                                                       GLSamplerObject::clearCache();}, GuiTheme::TOOL_BUTTON_STYLE);
    }

    pack();
}


shared_ptr<SettingsWindow> SettingsWindow::create(const shared_ptr<GuiTheme>& theme, GApp* app) {
    return createShared<SettingsWindow>(theme, app);
}


void SettingsWindow::setManager(WidgetManager *manager) {
    GuiWindow::setManager(manager);
    if (manager) {
        // Move to the upper right
        ///float osWindowWidth = (float)manager->window()->width();
        ///setRect(Rect2D::xywh(osWindowWidth - rect().width(), 40, rect().width(), rect().height()));
    }
}


}
