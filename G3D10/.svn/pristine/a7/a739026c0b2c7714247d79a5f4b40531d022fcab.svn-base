/**
  \file G3D-app.lib/source/GuiButton.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-app/GuiButton.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiPane.h"

namespace G3D {

GuiButton::GuiButton(GuiContainer* parent, const GuiButton::Callback& callback, const GuiText& text, GuiTheme::ButtonStyle style) : 
    GuiControl(parent, text), 
    m_down(false), m_callback(callback), m_style(style) {}


void GuiButton::render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const {
    (void)rd;
    if (m_visible) {
        theme->renderButton(m_rect, m_enabled && ancestorsEnabled, focused() || mouseOver(), 
                           m_down && mouseOver(), m_caption, (GuiTheme::ButtonStyle)m_style);
    }
}


bool GuiButton::onEvent(const GEvent& event) {
    switch (event.type) {
    case GEventType::MOUSE_BUTTON_DOWN:
        m_down = true;
        fireEvent(GEventType::GUI_DOWN);
        return true;
    
    case GEventType::MOUSE_BUTTON_UP:
        fireEvent(GEventType::GUI_UP);

        // Only trigger an action if the mouse was still over the control
        if (m_down && m_rect.contains(event.mousePosition())) {
            fireEvent(GEventType::GUI_ACTION);
        }

        m_down = false;
        return true;

    case GEventType::GUI_ACTION:
        if (event.gui.control == this) { m_callback.execute(); }

        // Don't consume the event...someone else may want to listen to it
        return false;
    }

    return false;
}

void GuiButton::setDown() {
    m_down = true;
}

void GuiButton::setUp() {
    m_down = false;
}

bool GuiButton::isDown() {
    return m_down;
}

}
