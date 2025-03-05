/**
  \file tools/viewer/VideoViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef VideoViewer_h
#define VideoViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class VideoViewer : public Viewer {
private:

    shared_ptr<VideoPlayer> m_player;

public:
	VideoViewer();
	virtual void onInit(const G3D::String& filename, App* app) override;
    virtual bool onEvent(const GEvent& e, App* app) override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface>>& surfaceArray) override {}
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D>>& surface2D) override;
};

#endif 
