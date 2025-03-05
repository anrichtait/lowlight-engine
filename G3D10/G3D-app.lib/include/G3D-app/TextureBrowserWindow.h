/**
  \file G3D-app.lib/include/G3D-app/TextureBrowserWindow.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_TextureBrowserWindow_h
#define GLG3D_TextureBrowserWindow_h

#include "G3D-base/platform.h"
#include "G3D-app/Widget.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiLabel.h"
#include "G3D-app/GuiTextureBox.h"
#include "G3D-app/GuiDropDownList.h"
#include "G3D-app/GuiButton.h"

namespace G3D {

class GApp;

/**
 Gui used by DeveloperWindow default for displaying all currently allocated textures.
 \sa G3D::DeveloperWindow, G3D::GApp
 */
class TextureBrowserWindow : public GuiWindow {
protected:

    /** Width of the texture browser */
    float                       m_browserWidth = 400;
    bool                        m_moveToDefaultLocation = true;

    GuiTextureBox*              m_textureBox;

    GApp*                       m_app;

    GuiPane*                    m_pane;

    Array<weak_ptr<Texture> >   m_textures;

    TextureBrowserWindow(GApp* app, const shared_ptr<Texture>& texture);

public:

    /** Refreshes m_textures only, and sets a list of texture names */
    void getTextureList(Array<String>& textureNames);

    void setTextureIndex(int index);

    virtual void setManager(WidgetManager* manager);

    static shared_ptr<TextureBrowserWindow> create(GApp* app, const shared_ptr<Texture>& texture = nullptr);

    GuiTextureBox* textureBox();

    void setTexture(const shared_ptr<Texture>& texture);

    void setWidth(const float width);

    void setMoveToDefaultLocation(const bool move) {
        m_moveToDefaultLocation = move;
    }
};

}

#endif
