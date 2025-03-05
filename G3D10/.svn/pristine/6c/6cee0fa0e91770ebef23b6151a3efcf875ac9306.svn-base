/**
  \file G3D-app.lib/include/G3D-app/GuiFrameBox.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_GuiFrameBox_h
#define G3D_GuiFrameBox_h

#include "G3D-base/platform.h"
#include "G3D-base/Pointer.h"
#include "G3D-app/GuiControl.h"
#include "G3D-app/GuiContainer.h"

namespace G3D {

class GuiWindow;
class GuiPane;
class GuiTextBox;
class GuiTheme;
class RenderDevice;

/**
 For editing coordinate frames
 */
class GuiFrameBox : public GuiContainer {
    friend class GuiPane;
    friend class GuiWindow;

protected:

    Pointer<CFrame>             m_value;
    Array<GuiControl*>          m_labelArray;
    GuiTextBox*                 m_textBox;
    bool                        m_allowRoll;

    String frame() const;
    void setFrame(const String& s);

public:

    /** For use when building larger controls out of GuiFrameBox.  
        For making a regular GUI, use GuiPane::addFrameBox. */
    GuiFrameBox
       (GuiContainer*          parent,
        const Pointer<CFrame>& value,
        bool                   allowRoll,
        GuiTheme::TextBoxStyle textBoxStyle);

    virtual void setRect(const Rect2D& rect) override;

    virtual void setEnabled(bool e) override;

    void render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const override;

    virtual void findControlUnderMouse(Vector2 mouse, GuiControl*& control) override;

    ~GuiFrameBox();
};

} // namespace G3D
#endif
