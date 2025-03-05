/**
  \file G3D-app.lib/include/G3D-app/GuiMultiLineTextBox.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#include "G3D-app/GuiTextBox.h"

namespace G3D {

/** Text box with multiple line support for entering strings. \sa GuiTextBox

    Enter adds a line break instead of firing GEventType::GUI_ACTION like GuiTextBox does.

    <b>Events:</b>
    <ol> 
      <li> GEventType::GUI_ACTION when the box loses focus
      <li> GEventType::GUI_CHANGE as text is entered (in IMMEDIATE_UPDATE mode)
      <li> GEventType::GUI_CANCEL when ESC is pressed
    </ol>
*/
class GuiMultiLineTextBox : public GuiTextBox {
protected:
    void moveCursorUp();
    void moveCursorDown();

    virtual void moveCursorHome() override;

    virtual void moveCursorEnd() override;

    virtual int coordsToCursorPos(Vector2 coords) override;

    virtual bool onEvent(const GEvent& event) override;

public:
    GuiMultiLineTextBox(GuiContainer* parent, const GuiText& caption, const Pointer<String>& value, Update update, GuiTheme::TextBoxStyle style);

    virtual void render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const override;
};

} // namesapce G3D