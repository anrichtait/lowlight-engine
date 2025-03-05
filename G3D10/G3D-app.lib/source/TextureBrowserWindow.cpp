/**
  \file G3D-app.lib/source/TextureBrowserWindow.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/TextureBrowserWindow.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/GuiTabPane.h"
#include "G3D-app/GApp.h"
#include "G3D-gfx/GLFWWindow.h"

namespace G3D {

static bool alphabeticalG3DFirst(weak_ptr<Texture> const& elem1, weak_ptr<Texture> const& elem2) {
    const shared_ptr<Texture>& elem1Locked = elem1.lock();
    const shared_ptr<Texture>& elem2Locked = elem2.lock();

    if (isNull(elem1Locked)) {
        return true;
    } else if (isNull(elem2Locked)) {
        return false;
    } else {
        return alphabeticalIgnoringCaseG3DFirstLessThan(elem1Locked->name(), elem2Locked->name());
    }
}

void TextureBrowserWindow::setTextureIndex(int index) {
    alwaysAssertM(index < m_textures.size(), "Texture and name list out of sync");
    const shared_ptr<Texture>& selectedTexture = m_textures[index].lock();
    setTexture(selectedTexture);
}


void TextureBrowserWindow::setTexture(const shared_ptr<Texture>& selectedTexture) {
    if (isNull(m_textureBox)) {
        m_textureBox = m_pane->addTextureBox(m_app);
    }

    if (notNull(selectedTexture)) {
        m_textureBox->setTexture(selectedTexture, false);
        m_textureBox->setCaption(selectedTexture->name());
    } else {
        m_textureBox->setTexture(nullptr, false);
    }

    setWidth(m_browserWidth);
}

void TextureBrowserWindow::setWidth(const float width) {

    float heightToWidthRatio = 0;
    const shared_ptr<Texture>& selectedTexture = m_textureBox->texture();
    if (notNull(selectedTexture)) {
        heightToWidthRatio = selectedTexture->height() / (float)selectedTexture->width();
    }

    m_textureBox->setSizeFromInterior(Vector2(width, width * heightToWidthRatio));
    m_textureBox->zoomToFit();

    pack();
}


void TextureBrowserWindow::getTextureList(Array<String>& textureNames) {
    m_textures.clear();
    textureNames.clear();
    Texture::getAllTextures(m_textures);
    m_textures.sort(alphabeticalG3DFirst);
    for (int i = 0; i < m_textures.size(); ++i) {
        const shared_ptr<Texture>& t = m_textures[i].lock();

        if (notNull(t) && t->appearsInTextureBrowserWindow()) {
            const String& displayedName = (! t->name().empty()) ? 
                                                t->name() : 
                                                format("Texture@0x%lx", (long int)(intptr_t)t.get());
            textureNames.append(displayedName);
        } else {
            m_textures.remove(i);
            --i; // Stay at current location
        }
    }
}


TextureBrowserWindow::TextureBrowserWindow
   (GApp*                                 app,
    const shared_ptr<Texture>&            texture) : 
    GuiWindow("Texture Browser", 
              app->developerWindow->theme(),
              Rect2D::xywh(5, 54, 200, 0),
              GuiTheme::FULL_DISAPPEARING_STYLE,
              GuiWindow::REMOVE_ON_CLOSE),
              m_textureBox(nullptr) {

    m_pane = GuiWindow::pane();
    m_app = app;
    pack();

    if (notNull(texture)) {
        setTexture(texture);
    }
}


shared_ptr<TextureBrowserWindow> TextureBrowserWindow::create(GApp* app, const shared_ptr<Texture>& texture) {
    return createShared<TextureBrowserWindow>(app, texture);
}


void TextureBrowserWindow::setManager(WidgetManager* manager) {
    GuiWindow::setManager(manager);
    if (manager) {

        // Move to the upper right if the window hasn't already been positioned.
        if (m_moveToDefaultLocation) {
            static const float scale = GLFWWindow::defaultGuiPixelScale();
            const float osWindowWidth = (float)manager->window()->width() / scale;
            const float osWindowHeight = (float)manager->window()->height() / scale;
            setRect(Rect2D::xywh(osWindowWidth * 0.9f - rect().width(), (osWindowHeight- rect().width()) / 2, rect().width(), rect().height()));
        }
    }
}


GuiTextureBox* TextureBrowserWindow::textureBox() {
    return m_textureBox;
}

}
