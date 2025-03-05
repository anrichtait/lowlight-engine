/**
  \file tools/viewer/IconSetViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef IconSetViewer_h
#define IconSetViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class IconSetViewer : public Viewer {
private:

    shared_ptr<GFont>   m_font;
    shared_ptr<IconSet> m_iconSet;

public:
    IconSetViewer(const shared_ptr<GFont>& captionFont);
    virtual void onInit(const String& filename, App* app) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;
};

#endif 
