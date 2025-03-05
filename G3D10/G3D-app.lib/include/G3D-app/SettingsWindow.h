/**
  \file G3D-app.lib/include/G3D-app/SettingsWindow.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_SettingsWindow_h

#include "G3D-base/platform.h"
#include "G3D-gfx/Profiler.h"
#include "G3D-app/Widget.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiLabel.h"
#include "G3D-app/GuiTextureBox.h"
#include "G3D-app/GuiDropDownList.h"
#include "G3D-app/GuiButton.h"

namespace G3D {

class  GApp;
    
/**
 \sa G3D::DeveloperWindow, G3D::GApp
 */
class SettingsWindow : public GuiWindow {
protected:

    GApp*           m_app;

    SettingsWindow(const shared_ptr<GuiTheme>& theme, GApp* app);

    // This MUST be in the header file and virtual so that it is compiled with the program
    virtual const String& compiledDate() const {
        static String c = __TIME__ ", " __DATE__;
        return c;
    }

    virtual const char* compiledBuild() const {
#       ifdef G3D_DEBUG
            return "Debug";
#       else
            return "Optimized";
#       endif
    }

public:

    virtual void setManager(WidgetManager* manager);

    static shared_ptr<SettingsWindow> create(const shared_ptr<GuiTheme>& theme, GApp* app);

};

} // G3D

