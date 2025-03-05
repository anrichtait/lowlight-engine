/**
  \file tools/viewer/EventViewer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef EventViewer_h
#define EventViewer_h

#include "G3D/G3D.h"
#include "Viewer.h"

class EventViewer : public Viewer {
private:
    Queue<String>       eventQueue;
    bool                m_lastEventWasMouseMove;

    shared_ptr<GFont>   font;

    bool                m_showMouseMoveEvents;


    /** Prints the event queue, flushing events that are not displayed
        on screen */
    void printEventQueue(RenderDevice* rd);

    void printWindowInformation(RenderDevice* rd);

    void printJoystickInformation(RenderDevice* rd);

public:

    EventViewer();
    virtual void onInit(const String& filename, App* app) override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) override;
    virtual bool onEvent(const GEvent& e, App* app) override;
};

#endif 
