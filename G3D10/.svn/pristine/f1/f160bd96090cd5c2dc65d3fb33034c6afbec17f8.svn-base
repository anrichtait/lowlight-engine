/**
  \file tools/viewer/FontViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef FontViewer_h
#define FontViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class FontViewer : public Viewer {
private:
    
    String                      m_fontfilename;
    shared_ptr<GFont>		m_font;
    shared_ptr<GFont>		m_basefont;

public:
    FontViewer(shared_ptr<GFont> base);
    virtual void onInit(const String& filename, App* app) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;
};

#endif 
