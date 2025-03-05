/**
  \file G3D-app.lib/include/G3D-app/GuiTextBox.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_GuiTextBox_h

#include "G3D-app/GuiControl.h"
#include "G3D-base/Pointer.h"

namespace G3D {

class GuiWindow;
class GuiPane;

/** Text box for entering strings.  

    <b>Events:</b>
    <ol> 
      <li> GEventType::GUI_ACTION when enter is pressed or the box loses focus
      <li> GEventType::GUI_CHANGE as text is entered (in IMMEDIATE_UPDATE mode)
      <li> GEventType::GUI_CANCEL when ESC is pressed
    </ol>
*/
class GuiTextBox : public GuiControl {
    friend class GuiWindow;
    friend class GuiPane;
public:
    /** IMMEDIATE_UPDATE - Update the string and fire a GUI_ACTION every time the text is changed
        DELAYED_UPDATE - Wait until the box loses focus to fire an event and update the string. */
    enum Update {IMMEDIATE_UPDATE, DELAYED_UPDATE};

protected:

    /** The string that this box is associated with. This may be out of date if
        editing and in DELAYED_UPDATE mode. */
    Pointer<String>         m_value;

    /** The value currently being set by the user. When in
        IMMEDIATE_UPDATE mode, this is continually synchronized with
        m_value.*/
    String                  m_userValue;

    /** Character position in m_userValue of the cursor */
    int                     m_cursorPos;

    /** Character position of the highlight start position. -1 when not highlighting. */
    int                     m_highlightPos;

    /** True if currently highlighting text with mouse button down. */
    bool                    m_highlighting;

    /** True if currently being edited, that is, if the user has
     changed the string more recently than the program has changed
     it.*/
    bool                    m_editing;

    /** Original value before the user started editing.  This is used
        to detect changes in m_value while the user is editing. */
    String                  m_oldValue;

    Update                  m_update;

    /** String to be used as the cursor character */
    GuiText                 m_cursor;

    /** Key that is currently auto-repeating. */
    GKeySym                 m_repeatKeysym;

    /** Time at which setRepeatKeysym was called. */
    RealTime                m_keyDownTime;

    /** Time at which the key will repeat (if down). */
    RealTime                m_keyRepeatTime;

    GuiTheme::TextBoxStyle  m_style;

    /** Called from onEvent when a key is pressed. */
    void setRepeatKeysym(GKeySym key);

    /** Called from onEvent when the repeat key is released. */
    void unsetRepeatKeysym();

    /** Called from render and onEvent to enact the action triggered by the repeat key. */
    void processRepeatKeysym();

    /** Returns begin and end indices of highlighted string in left-to-right order. */
    Vector2int32 highlightedRange();

    void stopHighlighting();

    void replaceHighlightedText(const String& s);

    /** Inserts character at cursor position and fires update events */
    void insertCharacter(char c);

    void backspaceCharacter();

    void deleteCharacter();

    void moveCursorLeft();

    void moveCursorRight();


    virtual int coordsToCursorPos(Vector2 coords);

    /** Called to change the value to the typed value.*/
    virtual void commit();

    virtual bool onEvent(const GEvent& event) override;

public:

    /** Moves cursor to home position (Home key). */
    virtual void moveCursorHome();

    /** Moves cursor to end position (End key). */
    virtual void moveCursorEnd();

    /** For use when building larger controls out of GuiNumberBox.  For making a regular GUI, use GuiPane::addTextBox. */
    GuiTextBox(GuiContainer* parent, const GuiText& caption, const Pointer<String>& value, Update update, GuiTheme::TextBoxStyle style);

    virtual void setRect(const Rect2D&) override;

    virtual void render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const override;

    /** Returns copy of highlighted text. */
    String highlightedText();
};

} // G3D
