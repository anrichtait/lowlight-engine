/**
  \file G3D-app.lib/include/G3D-app/GuiWidgetDestructor.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_GuiWidgetDestructor_h
#define GLG3D_GuiWidgetDestructor_h

#include "G3D-base/platform.h"
#include "G3D-app/GuiControl.h"

namespace G3D {

class Widget;
class GuiContainer;

/** Detects when this object is removed from the GUI and removes the corresponding widget from its manager. */
class GuiWidgetDestructor : public GuiControl {
protected:

    weak_ptr<Widget> m_widget;

public:

    GuiWidgetDestructor(GuiContainer* parent, const weak_ptr<Widget>& widget);

    virtual void render(RenderDevice* rd, const shared_ptr< GuiTheme >& theme, bool ancestorsEnabled) const override {}

    ~GuiWidgetDestructor();
};

} // namespace

#endif
