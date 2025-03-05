/**
  \file tools/viewer/TextureViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef TextureViewer_h
#define TextureViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class TextureViewer : public Viewer {
private:

	shared_ptr<Texture> m_texture;
	int				    m_width;
	int				    m_height;
    bool                m_isSky;

public:
	TextureViewer();
	virtual void onInit(const String& filename, App* app) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface>>& surfaceArray) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D>>& surface2D) override;

};

#endif 
