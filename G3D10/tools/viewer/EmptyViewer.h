/**
  \file tools/viewer/EmptyViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef EmptyViewer_h
#define EmptyViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class EmptyViewer : public Viewer {

public:
	EmptyViewer();
	virtual void onInit(const String& filename, App* app) override;
	virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;
};

#endif 
