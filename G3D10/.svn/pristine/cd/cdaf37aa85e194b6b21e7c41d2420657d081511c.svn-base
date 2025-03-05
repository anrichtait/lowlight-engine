/**
  \file G3D-app.lib/source/GuiLabel.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-app/GuiLabel.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiPane.h"

namespace G3D {

GuiLabel::GuiLabel(GuiContainer* parent, const GuiText& text, GFont::XAlign x, GFont::YAlign y) 
    : GuiControl(parent, text), m_xalign(x), m_yalign(y) {
}


void GuiLabel::render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const {
    (void)rd;
    if (m_visible) {
        theme->renderLabel(m_rect, m_caption, m_xalign, m_yalign, m_enabled && ancestorsEnabled, true);
    }
}


}
