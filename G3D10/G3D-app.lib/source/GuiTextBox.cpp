/**
  \file G3D-app.lib/source/GuiTextBox.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-app/GuiTextBox.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiPane.h"

/** Cursor flashes per second. */
static const float               blinkRate = 3.0f;

/** Keypresses per second. */
static const float               keyRepeatRate = 18.0f;

/** Delay before repeat begins. */
static const float               keyRepeatDelay = 0.25f;

namespace G3D {

GuiTextBox::GuiTextBox
(GuiContainer*               parent,
 const GuiText&              caption, 
 const Pointer<String>& value, 
 Update                      update,
 GuiTheme::TextBoxStyle      style) : 
    GuiControl(parent, caption), 
    m_value(value), 
    m_cursorPos(0), 
    m_highlightPos(-1),
    m_highlighting(false),
    m_editing(false), 
    m_update(update),
    m_cursor("|"),
    m_style(style) {

    unsetRepeatKeysym();
    m_keyDownTime = System::time();
}


void GuiTextBox::render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const {
    (void)rd;
    (void)theme;
    GuiTextBox* me = const_cast<GuiTextBox*>(this);

    if (m_visible) {
        if (m_editing) {
            if (! focused()) {
                // Just lost focus
                if ((m_update == DELAYED_UPDATE) && (m_oldValue != m_userValue)) {
                    me->m_oldValue = m_userValue;
                    me->commit();
                    GEvent response;
                    response.gui.type = GEventType::GUI_CHANGE;
                    response.gui.control = me->m_eventSource;
                    m_gui->fireEvent(response);
                }
                me->m_editing = false;
            } else if (String(*m_value) != m_oldValue) {
                // The value has been changed by the program while we
                // were editing; override our copy with the
                // programmatic value.
                me->m_userValue = *m_value;
                me->m_cursorPos = min(m_cursorPos, (int)m_userValue.size());
            }
        } else if (focused()) {
            // Just gained focus
            me->m_userValue = *m_value;
            me->m_oldValue  = m_userValue;
            me->m_editing   = true;
        }

        static RealTime then = System::time();
        RealTime now = System::time();

        bool hasKeyDown = (m_repeatKeysym.sym != GKey::UNKNOWN);

        // Amount of time that the last simulation step took.  
        // This is used to limit the key repeat rate
        // so that it is not faster than the frame rate.
        RealTime frameTime = then - now;

        // If a key is being pressed, process it on a steady repeat schedule.
        if (hasKeyDown && (now > m_keyRepeatTime)) {
            me->processRepeatKeysym();
            me->m_keyRepeatTime = max(now + frameTime * 1.1, now + 1.0 / keyRepeatRate);
        }
        then = now;

        // Only blink the cursor when keys are not being pressed or
        // have not recently been pressed.
        bool solidCursor = hasKeyDown || (now - m_keyRepeatTime < 1.0 / blinkRate);
        if (! solidCursor) {
            static const RealTime zero = System::time();
            solidCursor = isOdd((int)((now - zero) * blinkRate));
        }

        // Note that textbox does not have a mouseover state
        theme->renderTextBox
           (m_rect,
            m_enabled && ancestorsEnabled,
            focused(), 
            m_caption,
            m_captionWidth,
            m_editing ? m_userValue : *m_value, 
            solidCursor ? m_cursor : GuiText(),
            m_cursorPos,
            m_highlightPos,
            m_style);
    }
}

void GuiTextBox::setRepeatKeysym(GKeySym key) {
    m_keyDownTime = System::time();
    m_keyRepeatTime = m_keyDownTime + keyRepeatDelay;
    m_repeatKeysym = key;
}


void GuiTextBox::unsetRepeatKeysym() {
    m_repeatKeysym.sym = GKey::UNKNOWN;
}


void GuiTextBox::processRepeatKeysym() {
    debugAssert(m_cursorPos >= 0);
    debugAssert(m_cursorPos < 100000);

    switch (m_repeatKeysym.sym) {
    case GKey::UNKNOWN:
        // No key
        break;

    case GKey::RIGHT:
        moveCursorRight();
        break;

    case GKey::LEFT:
        moveCursorLeft();
        break;

    case GKey::HOME:
        moveCursorHome();
        break;

    case GKey::END:
        moveCursorEnd();
        break;

    case GKey::DELETE:
        deleteCharacter();
        break;

    case GKey::BACKSPACE:
        backspaceCharacter();
        break;

    default:
        
        // This key wasn't processed by the console
        debugAssertM(false, "Unexpected repeat key");
        
    }

    if (m_editing && (m_update == IMMEDIATE_UPDATE)) {
        m_oldValue = m_userValue;
        commit();
        GEvent response;
        response.gui.type = GEventType::GUI_CHANGE;
        response.gui.control = m_eventSource;
        m_gui->fireEvent(response);
    }
}

Vector2int32 GuiTextBox::highlightedRange() {
    if (m_highlightPos == -1) {
        return Vector2int32();
    }

    return (m_highlightPos > m_cursorPos) ?
        Vector2int32(m_cursorPos, m_highlightPos) :
        Vector2int32(m_highlightPos, m_cursorPos);
}

void GuiTextBox::stopHighlighting() {
    m_highlightPos = -1;
    m_highlighting = false;
}

String GuiTextBox::highlightedText() {
    const Vector2int32& range = highlightedRange();
    return m_userValue.substr(range.x, range.y - range.x);
}

void GuiTextBox::replaceHighlightedText(const String& s) {
    const Vector2int32& range = highlightedRange();
    m_userValue =
        m_userValue.substr(0, range.x) +
        s +
        m_userValue.substr(range.y);

    m_cursorPos = int(range.x + s.length());

    stopHighlighting();
}

void GuiTextBox::insertCharacter(char c) {
    if (m_highlightPos != -1) {
      replaceHighlightedText(String(1, c));
    } else {
        m_userValue =
            m_userValue.substr(0, m_cursorPos) +
            c +
            ((m_cursorPos < (int)m_userValue.size()) ?
                m_userValue.substr(m_cursorPos, String::npos) :
                String());
        ++m_cursorPos;
    }

    if (m_editing && (m_update == IMMEDIATE_UPDATE)) {
        m_oldValue = m_userValue;
        commit();
        GEvent response;
        response.gui.type = GEventType::GUI_CHANGE;
        response.gui.control = m_eventSource;
        m_gui->fireEvent(response);
    }
}

void GuiTextBox::backspaceCharacter() {
    if (m_highlightPos != -1) {
        replaceHighlightedText("");
    } else {
        if (m_cursorPos > 0) {
            m_userValue =
                m_userValue.substr(0, m_cursorPos - 1) +
                ((m_cursorPos < (int)m_userValue.size()) ?
                    m_userValue.substr(m_cursorPos, String::npos) :
                    String());
            --m_cursorPos;

            debugAssert(m_cursorPos <= (int)m_userValue.size());
        }
    }
}

void GuiTextBox::deleteCharacter() {
    if (m_highlightPos != -1) {
        replaceHighlightedText("");
    } else {
        if (m_cursorPos < (int)m_userValue.size()) {
            m_userValue =
                m_userValue.substr(0, m_cursorPos) +
                m_userValue.substr(m_cursorPos + 1, String::npos);

            debugAssert(m_cursorPos <= (int)m_userValue.size());
        }
    }
}

void GuiTextBox::moveCursorLeft() {
    stopHighlighting();
    if (m_cursorPos > 0) {
        --m_cursorPos;
    }
}

void GuiTextBox::moveCursorRight() {
    stopHighlighting();
    if (m_cursorPos < (int)m_userValue.size()) {
        ++m_cursorPos;
    }
}

void GuiTextBox::moveCursorHome() {
    stopHighlighting();
    m_cursorPos = 0;
}

void GuiTextBox::moveCursorEnd() {
    stopHighlighting();
    m_cursorPos = (int)m_userValue.size();
}

int GuiTextBox::coordsToCursorPos(Vector2 coords) {
    int outputCursorPos = 0;

    const GuiTheme::TextStyle& style = theme()->defaultStyle();
    //Offset if some text is hidden to the left
    const float textOffset = max(0.0f, style.font->bounds(m_userValue.substr(0, m_cursorPos), style.size).x - (clickRect().width() - theme()->textBoxPadding().wh().x));

    const float clickPos = coords.x - (m_captionWidth + rect().corner(0).x) + textOffset - theme()->textBoxPadding().topLeft.x;

    for (size_t i = 1; i < m_userValue.size(); ++i) {         
        float newPos = style.font->bounds(m_userValue.substr(0, i), style.size).x;
        if (newPos > clickPos) {
            //subtracting one places the cursor before a clicked letter, as is normal
            outputCursorPos = (int)i - 1;
            break;
        }
        if (i == m_userValue.size() - 1) {
            //if the cursor pos is not otherwise set, put it at the end
            outputCursorPos = (int)m_userValue.size();
        }
    }
    return outputCursorPos;
}

void GuiTextBox::commit() {
    *m_value = m_userValue;
}


bool GuiTextBox::onEvent(const GEvent& event) {
    if (! m_visible) {
        return false;
    }

    switch (event.type) {
    case GEventType::MOUSE_BUTTON_DOWN:
        /**place the cursor in the space closest to the mouse click (does not support multi-line textboxes)*/
        m_cursorPos = coordsToCursorPos(Vector2(event.button.x, event.button.y));
        m_highlightPos = -1;
        m_highlighting = true;
        break;

    case GEventType::MOUSE_MOTION:
        if (m_highlighting) {
            m_highlightPos = coordsToCursorPos(Vector2(event.motion.x, event.motion.y));
            if (m_highlightPos == m_cursorPos) {
                m_highlightPos = -1;
            }
        }
        break;

    case GEventType::MOUSE_BUTTON_UP:
        m_highlighting = false;
        break;

    case GEventType::GUI_KEY_FOCUS:
        if ((event.gui.control == this) && (m_gui->keyFocusGuiControl != this) && m_editing) {
            // This box just lost focus
            commit();
            m_highlightPos = -1;
            m_highlighting = false;
            m_editing = false;
            if (m_update == DELAYED_UPDATE) {
                GEvent response;
                response.gui.type = GEventType::GUI_CHANGE;
                response.gui.control = m_eventSource;
                m_gui->fireEvent(response);
            }
        }
        break;

    case GEventType::KEY_DOWN:
    case GEventType::KEY_REPEAT:
        switch (event.key.keysym.sym) {
        case GKey::ESCAPE:
            // Stop editing and revert
            m_highlightPos = -1;
            m_highlighting = false;
            m_editing = false;
            {
                GEvent response;
                response.gui.type = GEventType::GUI_CANCEL;
                response.gui.control = m_eventSource;
                m_gui->fireEvent(response);
            }            
            setFocused(false);
            return true;

        case GKey::RIGHT:
        case GKey::LEFT:
        case GKey::DELETE:
        case GKey::BACKSPACE:
        case GKey::HOME:
        case GKey::END:
            setRepeatKeysym(event.key.keysym);
            processRepeatKeysym();
            debugAssert(m_cursorPos <= (int)m_userValue.size());
            return true;

        case GKey::RETURN:
        case GKey::TAB:
            // Edit done, commit.
            m_highlightPos = -1;
            m_highlighting = false;

            setFocused(false);
            if (event.key.keysym.sym == GKey::RETURN) {
                commit();
                GEvent response;
                response.gui.type = GEventType::GUI_ACTION;
                response.gui.control = m_eventSource;
                m_gui->fireEvent(response);
            }
            return true;

        default:

            if ((((event.key.keysym.mod & GKeyMod::CTRL) != 0) &&
                 ((event.key.keysym.sym == 'v') || (event.key.keysym.sym == 'y'))) ||

                (((event.key.keysym.mod & GKeyMod::SHIFT) != 0) &&
                (event.key.keysym.sym == GKey::INSERT))) {

                // Paste (not autorepeatable)
                m_userValue = OSWindow::clipboardText();
                return true;

            } else if (((event.key.keysym.mod & GKeyMod::CTRL) != 0) && (event.key.keysym.sym == 'k')) {

                debugAssert(m_cursorPos <= (int)m_userValue.size());

                // Cut (not autorepeatable)
                const String& cut = m_userValue.substr(m_cursorPos);
                m_userValue = m_userValue.substr(0, m_cursorPos);

                OSWindow::setClipboardText(cut);

                return true;

            } else if ((event.key.keysym.sym >= GKey::SPACE) && (event.key.keysym.sym <= 'z')) {
                // Suppress this event. Actually handle the key press on the CHAR_INPUT event
                return true;
            } else {
                // This key wasn't processed by the console
                return false;
            }
        }
        break;

    case GEventType::KEY_UP:
        if (event.key.keysym.sym == m_repeatKeysym.sym) {
            unsetRepeatKeysym();
            return true;
        }
        break;

    case GEventType::CHAR_INPUT:
        if (event.character.unicode > 31) {
            // Insert character
            const char c = event.character.unicode & 0xFF;
            insertCharacter(c);
        }
        break;
    }

    return false;
}


void GuiTextBox::setRect(const Rect2D& rect) {
     m_rect = rect;
     m_clickRect = theme()->textBoxToClickBounds(rect, m_captionWidth);
}


}
