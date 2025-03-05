/**
  \file G3D-app.lib/include/G3D-app/GuiLabel.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef G3D_GUILABEL_H
#define G3D_GUILABEL_H

#include "G3D-app/GuiControl.h"

namespace G3D {

class GuiWindow;
class GuiPane;

/** A text label.  Will never be the focused control and will never
    have the mouse over it. */
class GuiLabel : public GuiControl {
private:
    friend class GuiWindow;
    friend class GuiPane;

    GFont::XAlign m_xalign;
    GFont::YAlign m_yalign;
    
    void render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const;

public:

    GuiLabel(GuiContainer*, const GuiText&, GFont::XAlign, GFont::YAlign);

    void setXAlign(GFont::XAlign a) {
        m_xalign = a;
    }

    GFont::XAlign xAlign() const {
        return m_xalign;
    }

    void setYAlign(GFont::YAlign a) {
        m_yalign = a;
    }

    GFont::YAlign yAlign() const {
        return m_yalign;
    }
};

} // G3D

#endif
