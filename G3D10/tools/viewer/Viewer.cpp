/**
\file tools/viewer/Viewer.cpp

G3D Innovation Engine http://casual-effects.com/g3d
Copyright 2000-2022, Morgan McGuire
All rights reserved
Available under the BSD License
*/

#include "Viewer.h"

extern App* app;


Viewer3D::Viewer3D(bool useSkyboxSurface) {
    if (isNull(m_instructions)) {
        m_instructions  = InstructionSurface::create(Texture::fromFile(System::findDataFile("keyguide.png"), ImageFormat::RGBA8(), Texture::DIM_2D), GFont::fromFile(System::findDataFile("arial.fnt")));
    }
    
    if (useSkyboxSurface) {
        m_skyboxSurface = SkyboxSurface::create(Texture::fromFile(FilePath::concat(System::findDataFile("whiteroom"), "whiteroom-*.png"), ImageFormat::SRGB8(), Texture::DIM_CUBE_MAP));
    }
}


shared_ptr<Viewer3D::InstructionSurface> Viewer3D::InstructionSurface::create(const shared_ptr<Texture>& guide, const shared_ptr<GFont>& font) {
    return createShared<InstructionSurface>(guide, font);
}


Rect2D Viewer3D::InstructionSurface::bounds() const {
    return Rect2D::xywh(0, 0, (float)m_guide->width(), (float)m_guide->height());
}


float Viewer3D::InstructionSurface::depth() const {
    // Lowest possible depth
    return 0.0f;
}


void Viewer3D::InstructionSurface::render(RenderDevice *rd) const {
    const Rect2D& rect = 
        Rect2D::xywh(15.0f, float(rd->height() - m_guide->height() - 5), 
            (float)m_guide->width(), (float)m_guide->height());

    m_font->draw2D(rd, "ESC - Quit  F3 - Toggle Hierarchy  F4 - Screenshot   F6 - Record Video   F8 - Render Cube Map   R - Reload", rect.x0y0() + Vector2(-10, -25), 10, Color3::black(), Color3::white());
    rd->setBlendFunc(RenderDevice::BLEND_SRC_ALPHA, RenderDevice::BLEND_ONE_MINUS_SRC_ALPHA);
    Draw::rect2D(rect, rd, Color4(Color3::white(), 0.8f), m_guide);
}


void Viewer3D::onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    if (m_skyboxSurface) posed3D.append(m_skyboxSurface);
    if (app->showInstructions) {
        posed2D.append(m_instructions);
    }
}

void Viewer3D::onGraphics3D(RenderDevice * rd, Array<shared_ptr<Surface> >&surfaceArray) {
    float x, y, z, yaw, pitch, roll;
    app->activeCamera()->frame().getXYZYPRDegrees(x, y, z, yaw, pitch, roll);
    screenPrintf("[Camera position: Translation(%f, %f, %f) Rotation(%f, %f, %f)]\n", x, y, z, yaw, pitch, roll);
}

bool Viewer3D::onEvent(const GEvent& e, App* app) {
    if ((e.type == GEventType::KEY_DOWN) && (e.key.keysym.sym == 'r')) {
        onInit(m_filename, app);
        return true;
    }
    return false;
}