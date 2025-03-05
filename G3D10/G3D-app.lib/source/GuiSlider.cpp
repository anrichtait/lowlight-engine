/**
  \file G3D-app.lib/source/GuiSlider.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-app/GuiSlider.h"
#include "G3D-app/GuiWindow.h"

namespace G3D {

_GuiSliderBase::_GuiSliderBase(GuiContainer* parent, const GuiText& text, bool horizontal) :
    GuiControl(parent, text), 
    m_horizontal(horizontal),
    m_inDrag(false) {
}


void _GuiSliderBase::render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const {
    (void)rd;
    if (m_visible && m_horizontal) {
        theme->renderHorizontalSlider(m_rect, floatValue(), m_enabled && ancestorsEnabled, focused() || mouseOver(), m_caption, m_captionWidth);
    }
}


bool _GuiSliderBase::onEvent(const GEvent& event) {
    if (! m_visible) {
        return false;
    }

    if (event.type == GEventType::MOUSE_BUTTON_DOWN) {
        Vector2 mouse(event.button.x, event.button.y);

        float v = floatValue();
        Rect2D thumbRect = theme()->horizontalSliderToThumbBounds(m_rect, v, m_captionWidth);
        Rect2D trackRect = theme()->horizontalSliderToTrackBounds(m_rect, m_captionWidth);
        
        if (thumbRect.contains(mouse)) {
            // Begin drag
            m_inDrag = true;
            m_dragStart = mouse;
            m_dragStartValue = v;

            GEvent response;
            response.gui.type = GEventType::GUI_DOWN;
            response.gui.control = m_eventSource;
            m_gui->fireEvent(response);

            response.gui.type = GEventType::GUI_CHANGE;
            response.gui.control = m_eventSource;
            m_gui->fireEvent(response);

            return true;
        } else if (trackRect.contains(mouse)) {
            // Jump to this position
            float p = clamp((mouse.x - trackRect.x0()) / trackRect.width(), 0.0f, 1.0f);
            setFloatValue(p);
            m_inDrag = false;

            GEvent response;
            response.gui.type = GEventType::GUI_CHANGE;
            response.gui.control = m_eventSource;
            m_gui->fireEvent(response);

            fireEvent(GEventType::GUI_ACTION);
            return true;
        }

    } else if (event.type == GEventType::MOUSE_BUTTON_UP) {
        if (m_inDrag) {
            // End the drag
            m_inDrag = false;

            fireEvent(GEventType::GUI_DOWN);
            fireEvent(GEventType::GUI_ACTION);

            return true;
        }

    } else if (m_inDrag && (event.type == GEventType::MOUSE_MOTION)) {
        // We'll only receive these events if we have the keyFocus, but we can't
        // help receiving the key focus if the user clicked on the control!

        const Vector2 mouse(event.motion.x, event.motion.y);
        const Rect2D& trackRect = theme()->horizontalSliderToTrackBounds(m_rect, m_captionWidth);

        const float delta = (mouse.x - m_dragStart.x) / trackRect.width();
        const float p = clamp(m_dragStartValue + delta, 0.0f, 1.0f);
        setFloatValue(p);

        GEvent response;
        response.gui.type = GEventType::GUI_CHANGE;
        response.gui.control = m_eventSource;
        m_gui->fireEvent(response);

        return true;
    }
    return false;
}

}
