/**
  \file tools/viewer/VideoViewer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "VideoViewer.h"


VideoViewer::VideoViewer() {
}


void VideoViewer::onInit(const G3D::String& filename, App* app) {
    m_player = VideoPlayer::fromFile(filename);
}

bool VideoViewer::onEvent(const GEvent& e, App* app) {
    if (e.type == GEventType::MOUSE_BUTTON_CLICK) {
        if (notNull(m_player)) {
            if (m_player->paused()) {
                m_player->unpause();
            } else {
                m_player->pause();
            }
        }
    }
    return false;
}

void VideoViewer::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    if (notNull(m_player)) {
        if (! m_player->finished()) {
            m_player->update(rdt);
        }
    }
}

void VideoViewer::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) {
    if (notNull(m_player->frameTexture())) {
        Draw::rect2D(rd->viewport().largestCenteredSubRect(float(m_player->frameTexture()->width()), float(m_player->frameTexture()->height())),
	        rd, Color3::white(), m_player->frameTexture());
    }

    if (m_player) {
        screenPrintf("Video: %d x %d", m_player->width(), m_player->height());
    } else {
        screenPrintf("Video: not supported");
    }

    screenPrintf("Window: %d x %d", rd->width(), rd->height());
    screenPrintf("Click to Pause/Unpause");
}
