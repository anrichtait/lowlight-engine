/**
  \file G3D-app.lib/include/G3D-app/GuiTabPane.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_GuiTabPane_h
#define GLG3D_GuiTabPane_h

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/Pointer.h"
#include "G3D-app/GuiContainer.h"
#include "G3D-app/GuiDropDownList.h"

namespace G3D {

class GuiPane;
    /**
    A gui control that allows the user to switch between a variety of differnt panes. 
    The panes are represented by tabs if they fit on the screen in which case it switches 
    to a drop down window.
    **/
class GuiTabPane : public GuiContainer {
protected:

    /** Used if no index pointer is provided */
    int                 m_internalIndex;
    GuiPane*            m_tabButtonPane;
    GuiDropDownList*    m_tabDropDown;
    GuiPane*            m_viewPane;

    /** Parallel array to m_contentPaneArray */
    Array<int>          m_contentIDArray;
    Array<GuiPane*>     m_contentPaneArray;
    Pointer<int>        m_idPtr;

    /** Events are only delivered to a control when the control that
        control has the key focus (which is transferred during a mouse
        down) */
    virtual bool onEvent(const GEvent& event) override { (void)event; return false; }

public:

    /** For use by GuiPane.  Call GuiPane::addTabPane to create */
    GuiTabPane(GuiContainer* parent, const Pointer<int>& id = nullptr);

    /** \param id If -1 (default), set to the integer corresponding to the number of panes already in existence. Useful 
        for cases where you want the index to correspond to an enum.*/
    GuiPane* addTab(const GuiText& label, int id = -1);

    virtual void findControlUnderMouse(Vector2 mouse, GuiControl*& control) override;
    virtual void render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const override;
    virtual void setRect(const Rect2D& rect) override;
    virtual void pack();

    void setSelectedIndex(int p) {
        *m_idPtr = p;
    }

    int selectedIndex() const {
        return *m_idPtr;
    }
};

}

#endif
