/**
  \file G3D-app.lib/source/GuiWidgetDestructor.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/GuiWidgetDestructor.h"
#include "G3D-app/GuiContainer.h"
#include "G3D-app/Widget.h"

namespace G3D {

GuiWidgetDestructor::GuiWidgetDestructor(GuiContainer* parent, const weak_ptr<Widget>& widget) : GuiControl(parent), m_widget(widget) {
    setSize(0, 0);
    setVisible(false);
    setEnabled(false);
}


GuiWidgetDestructor::~GuiWidgetDestructor() {
    const shared_ptr<Widget>& w = m_widget.lock();
    if (notNull(w) && notNull(w->manager())) {
        w->manager()->remove(w);
    }
}

}  // G3D
