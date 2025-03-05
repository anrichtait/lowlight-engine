/**
  \file G3D-app.lib/include/G3D-app/ProfilerWindow.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#ifndef GLG3D_ProfilerWindow_h
#define GLG3D_ProfilerWindow_h

#include "G3D-base/platform.h"
#include "G3D-gfx/Profiler.h"
#include "G3D-app/Widget.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiLabel.h"
#include "G3D-app/GuiTextureBox.h"
#include "G3D-app/GuiDropDownList.h"
#include "G3D-app/GuiButton.h"


namespace G3D {


/**
 \sa G3D::DeveloperWindow, G3D::GApp
 */
class ProfilerWindow : public GuiWindow {
protected:

    /** Inserted into the scroll pane */
    class ProfilerTreeDisplay : public GuiControl {
    public:

        bool collapsedIfIncluded;

        bool checkIfCollapsed(const size_t hash) const;

        void expandAll();

        void collapseAll();

        shared_ptr<GFont> m_icon;
        Set<size_t> m_collapsed;
        
        size_t m_selected;

        virtual bool onEvent(const GEvent& event) override;

        ProfilerTreeDisplay(GuiWindow* w);

        virtual void render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const override;
    };

    GuiScrollPane*              m_scrollPane;
    ProfilerTreeDisplay*        m_treeDisplay;

    void collapseAll();

    void expandAll();

    ProfilerWindow(const shared_ptr<GuiTheme>& theme);

public:

    virtual void setManager(WidgetManager* manager);

    static shared_ptr<ProfilerWindow> create(const shared_ptr<GuiTheme>& theme);

};

} // G3D

#endif
