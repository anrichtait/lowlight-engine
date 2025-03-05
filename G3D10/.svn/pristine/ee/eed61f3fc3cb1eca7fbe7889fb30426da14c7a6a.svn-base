/**
  \file G3D-app.lib/include/G3D-app/DebugTextWidget.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_DebugTextWidget_h
#define GLG3D_DebugTextWidget_h

#include "G3D-app/Widget.h"

namespace G3D {

/** Renders rasterizer statistics and debug text for GApp */
class DebugTextWidget : public FullScreenWidget {
    friend class GApp;
protected:

    GApp*           m_app;

    DebugTextWidget(GApp* app) : m_app(app) {}

public:

    static shared_ptr<DebugTextWidget> create(GApp* app) {
        return shared_ptr<DebugTextWidget>(new DebugTextWidget(app));
    }

    virtual void render(RenderDevice* rd) const override;
};

} // G3D

#endif
