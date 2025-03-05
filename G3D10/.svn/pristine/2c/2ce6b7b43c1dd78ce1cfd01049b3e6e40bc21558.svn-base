/**
  \file G3D-app.lib/source/GuiMultiLineTextBox.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/GuiMultiLineTextBox.h"
#include "G3D-app/GuiTextBox.h"
#include "G3D-app/GuiWindow.h"

namespace G3D {

/** Cursor flashes per second. */
static const float               blinkRate = 3.0f;

/** Keypresses per second. */
static const float               keyRepeatRate = 18.0f;

/** Delay before repeat begins. */
//static const float               keyRepeatDelay = 0.25f;


GuiMultiLineTextBox::GuiMultiLineTextBox(
    GuiContainer* parent, const GuiText& caption, const Pointer<String>& value, GuiTextBox::Update update, GuiTheme::TextBoxStyle style)
    : GuiTextBox(parent, caption, value, update, style) {

}

void GuiMultiLineTextBox::moveCursorUp() {
    stopHighlighting();
    const Array<String>& lines = splitLines(m_editing ? m_userValue : *m_value);

    // Find line containing cursor
    int cursorLine = 0;
    int cursorPosition = m_cursorPos;
    for (int c = 0; cursorLine < lines.length(); ++cursorLine) {
        const int countUpToThisLine = c;
        c += (int)lines[cursorLine].length() + 1;
        if (cursorPosition < c) {
            cursorPosition -= countUpToThisLine;
            break;
        }
    }

    // Quit if on first line
    if (cursorLine == 0) {
        return;
    }

    // Calculate the cursor position on cursor line
    const GuiTheme::TextStyle& style = theme()->defaultStyle();
    const String& beforeCursor = lines[cursorLine].substr(0, cursorPosition);
    const Vector2& beforeBounds = style.font->bounds(beforeCursor, style.size);

    // Calculate cursor position on previous line at same offset
    const String& prevLine = lines[cursorLine - 1];
    for (size_t i = 0; i < prevLine.size(); ++i) {         
        float newPos = style.font->bounds(prevLine.substr(0, i), style.size).x;
        if (newPos > beforeBounds.x) {
            int cursorPos = 0;
            for (int l = 0; l < (cursorLine - 1); ++l) {
                cursorPos += (int)lines[l].length() + 1;
            }

            // Subtract one to place at begining of character
            m_cursorPos = cursorPos + (int)i - 1;
            break;
        }

        if (i == (prevLine.size() - 1)) {
            int cursorPos = 0;
            for (int l = 0; l < (cursorLine - 1); ++l) {
                cursorPos += (int)lines[l].length() + 1;
            }

            // Subtract one to place at begining of character
            m_cursorPos = cursorPos + (int)prevLine.size();
        }
    } 
}

void GuiMultiLineTextBox::moveCursorDown() {
    stopHighlighting();
    const Array<String>& lines = splitLines(m_editing ? m_userValue : *m_value);

    // Find line containing cursor
    int cursorLine = 0;
    int cursorPosition = m_cursorPos;
    for (int c = 0; cursorLine < lines.length(); ++cursorLine) {
        const int countUpToThisLine = c;
        c += (int)lines[cursorLine].length() + 1;
        if (cursorPosition < c) {
            cursorPosition -= countUpToThisLine;
            break;
        }
    }

    // Quit if on last line
    if (cursorLine == (lines.length() - 1)) {
        return;
    }

    // Calculate the cursor position on cursor line
    const GuiTheme::TextStyle& style = theme()->defaultStyle();
    const String& beforeCursor = lines[cursorLine].substr(0, cursorPosition);
    const Vector2& beforeBounds = style.font->bounds(beforeCursor, style.size);

    // Calculate cursor position on next line at same offset
    const String& nextLine = lines[cursorLine + 1];
    for (size_t i = 0; i < nextLine.size(); ++i) {         
        float newPos = style.font->bounds(nextLine.substr(0, i), style.size).x;
        if (newPos > beforeBounds.x) {
            int cursorPos = 0;
            for (int l = 0; l <= cursorLine; ++l) {
                cursorPos += (int)lines[l].length() + 1;
            }

            m_cursorPos = cursorPos + (int)i - 1;
            break;
        }
        if (i == (nextLine.size() - 1)) {
            int cursorPos = 0;
            for (int l = 0; l <= cursorLine; ++l) {
                cursorPos += (int)lines[l].length() + 1;
            }

            m_cursorPos = cursorPos + (int)nextLine.size();
        }
    } 
}

void GuiMultiLineTextBox::moveCursorHome() {
    stopHighlighting();
    const Array<String>& lines = splitLines(m_editing ? m_userValue : *m_value);

    // Find line containing cursor
    int cursorLine = 0;
    int cursorPosition = m_cursorPos;
    for (int c = 0; cursorLine < lines.length(); ++cursorLine) {
        const int countUpToThisLine = c;
        c += (int)lines[cursorLine].length() + 1;
        if (cursorPosition < c) {
            cursorPosition -= countUpToThisLine;
            break;
        }
    }

    m_cursorPos = max(m_cursorPos - cursorPosition, 0);
}

void GuiMultiLineTextBox::moveCursorEnd() {
    stopHighlighting();
    const Array<String>& lines = splitLines(m_editing ? m_userValue : *m_value);

    // Find line containing cursor
    int cursorLine = 0;
    int cursorPosition = m_cursorPos;
    for (int c = 0; cursorLine < lines.length(); ++cursorLine) {
        const int countUpToThisLine = c;
        c += (int)lines[cursorLine].length() + 1;
        if (cursorPosition < c) {
            cursorPosition -= countUpToThisLine;
            break;
        }
    }

    const int maxPos = (int)(m_editing ? m_userValue : *m_value).length();
    m_cursorPos = min(m_cursorPos + ((int)lines[cursorLine].length() - cursorPosition), maxPos);
}

int GuiMultiLineTextBox::coordsToCursorPos(Vector2 coords) {
    int outputCursorPos = 0;
    const Array<String>& lines = splitLines(m_editing ? m_userValue : *m_value);
    if (lines.length() == 0) {
        return false;
    }

    // Find line containing cursor
    int cursorLine = 0;
    int cursorPosition = m_cursorPos;
    for (int c = 0; cursorLine < lines.length(); ++cursorLine) {
        const int countUpToThisLine = c;
        c += (int)lines[cursorLine].length() + 1;
        if (cursorPosition < c) {
            cursorPosition -= countUpToThisLine;
            break;
        }
    }

    // Area in which text appears
    const Rect2D& bounds = theme()->textBoxToClickBounds(m_rect, m_captionWidth);    
    Rect2D clientArea = Rect2D::xywh(bounds.x0y0() + theme()->textBoxPadding().topLeft, 
                                        bounds.wh() - (theme()->textBoxPadding().bottomRight + theme()->textBoxPadding().topLeft));

    // Calculate maximum lines that fit in client area
    const float lineHeight = theme()->defaultStyle().font->bounds(" ", theme()->defaultStyle().size).y;
    const int linesPerArea = iCeil(clientArea.height() / lineHeight);

    // Calculate the cursor position on cursor line
    const GuiTheme::TextStyle& style = theme()->defaultStyle();
    const String& beforeCursor = lines[cursorLine].substr(0, cursorPosition);
    const Vector2& beforeBounds = style.font->bounds(beforeCursor, style.size);
    const float textOffset = -max(0.0f, beforeBounds.x - clientArea.width());

    // Calculate start of lines to render within client area based on cursor line
    // cursorLine is 0-based to add 1 so lines are shifted after linesPerArea
    const int startingLine = max(cursorLine - linesPerArea + 1, 0);
    const int endLine = min(startingLine + linesPerArea, lines.length() - 1);

    // Calculate click line based on line height
    const int clickLine = max(0, min(startingLine + iFloor((coords.y - clientArea.y0()) / lineHeight), endLine));
    const float clickX = coords.x - clientArea.x0() + textOffset;

    // Calculate cursor position on click line
    const String& newLine = lines[clickLine];
    for (size_t i = 0; i < newLine.size(); ++i) {         
        float newPos = style.font->bounds(newLine.substr(0, i), style.size).x;
        if (newPos > clickX) {
            int cursorPos = 0;
            for (int l = 0; l < clickLine; ++l) {
                cursorPos += (int)lines[l].length() + 1;
            }

            // Subtract one to place at begining of character
            outputCursorPos = cursorPos + (int)i - 1;
            break;
        }

        if (i == (newLine.size() - 1)) {
            int cursorPos = 0;
            for (int l = 0; l < clickLine; ++l) {
                cursorPos += (int)lines[l].length() + 1;
            }

            // Subtract one to place at begining of character
            outputCursorPos = cursorPos + (int)newLine.size();
        }
    } 
    return outputCursorPos;
}

bool GuiMultiLineTextBox::onEvent(const GEvent& event) {
    if (!m_visible) {
        return false;
    }

    switch (event.type) {
    case GEventType::KEY_DOWN:
    case GEventType::KEY_REPEAT:
        switch (event.key.keysym.sym) {
        case GKey::UP:
            moveCursorUp();
            return true;

        case GKey::DOWN:
            moveCursorDown();
            return true;

        case GKey::RETURN:
            insertCharacter('\n');
            return true;
        default:
            break;
        }
    default:
        break;
    }

    return GuiTextBox::onEvent(event);
}

void GuiMultiLineTextBox::render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const {
    (void)rd;
    (void)theme;
    GuiMultiLineTextBox* me = const_cast<GuiMultiLineTextBox*>(this);

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
        theme->renderMultiLineTextBox
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

} // namesapce G3D
