/**
  \file G3D-app.lib/source/GuiDropDownList.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-app/GuiDropDownList.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiPane.h"

namespace G3D {


GuiDropDownList::GuiDropDownList
(GuiContainer*                         parent, 
 const GuiText&                        caption, 
 const Pointer<int>&                   indexValue, 
 const Array<GuiText>&                 listValue,
 const Pointer<Array<String> >&        listValuePtr,
 const GuiControl::Callback&           actionCallback,
 bool                                  usePrefixTreeMenus) :
 GuiControl(parent, caption), 
    m_indexValue(indexValue.isNull() ? Pointer<int>(&m_myInt) : indexValue),
    m_myInt(0),
    m_listValuePtr(listValuePtr),
    m_listValue(listValue),
    m_selecting(false),
    m_actionCallback(actionCallback),
    m_usePrefixTreeMenus(usePrefixTreeMenus) {
    menu();
}


shared_ptr<GuiMenu> GuiDropDownList::menu() { 
    if (isNull(m_menu)) {
         m_menu = GuiMenu::create(theme(), &m_listValue, m_indexValue, m_usePrefixTreeMenus,m_usePrefixTreeMenus);
    }
    return m_menu;
}


bool GuiDropDownList::containsValue(const String& s) const {
    if (m_usePrefixTreeMenus){
        return m_menu->containsValue(s);
    } else{
        for (int i = 0; i < m_listValue.size(); ++i) {
            if (m_listValue[i].text() == s) {
                return true;
            }
        }
    return false;
    }
}


void GuiDropDownList::render(RenderDevice* rd, const shared_ptr<GuiTheme>& theme, bool ancestorsEnabled) const {
    (void)rd;
    if (m_visible) {
        if (m_usePrefixTreeMenus){
         theme->renderDropDownList(m_rect, m_enabled && ancestorsEnabled, focused() || mouseOver(), 
                                 false, selectedValue(), m_caption, m_captionWidth);
        } else {
        theme->renderDropDownList(m_rect, m_enabled && ancestorsEnabled, focused() || mouseOver(), 
                                 m_selecting, selectedValue(), m_caption, m_captionWidth);
        }
    }

}


void GuiDropDownList::setSelectedValue(const String& s) {
    if (m_usePrefixTreeMenus){
        m_menu->setSelectedValue(s);
    } else {
        for (int i = 0; i < m_listValue.size(); ++i) {
            if (m_listValue[i].text() == s) {
                setSelectedIndex(i);
                return;
            }
        }
    }
}


void GuiDropDownList::showMenu() {
    Rect2D clickRect = theme()->dropDownListToClickBounds(rect(), m_captionWidth);
    Vector2 clickOffset = clickRect.x0y0() - rect().x0y0();
    Vector2 menuOffset(1,clickRect.height()+10);
    Vector2 menuPosition = toOSWindowCoords(clickOffset + menuOffset);

    menu()->show(m_gui->manager(), window(), this, menuPosition, false, m_actionCallback);
}


bool GuiDropDownList::onEvent(const GEvent& event) {
    if (! m_visible) {
         return false;
    }
    if (m_usePrefixTreeMenus){
         if (event.type == GEventType::MOUSE_BUTTON_DOWN) {
             if (! m_gui->manager()->contains(m_menu)) {
                 showMenu();
             }
            return true;
         }
    } else {
         if (event.type == GEventType::MOUSE_BUTTON_DOWN) {

             if (! m_gui->manager()->contains(menu())) {
                 showMenu();
             }
             return true;

         } else if (event.type == GEventType::KEY_DOWN) {
             switch (event.key.keysym.sym) {
             case GKey::DOWN:
                 *m_indexValue = selectedIndex();
                 if (*m_indexValue < m_listValue.size() - 1) {
                     *m_indexValue = *m_indexValue + 1;
                 } else {
                     *m_indexValue = 0;
                 }
                 m_actionCallback.execute();
                 fireEvent(GEventType::GUI_ACTION);
                 return true;

             case GKey::UP:
                 *m_indexValue = selectedIndex();
                 if (*m_indexValue > 0) {
                     *m_indexValue = *m_indexValue - 1;
                 } else {
                     *m_indexValue = m_listValue.size() - 1;
                 }
                 m_actionCallback.execute();
                 fireEvent(GEventType::GUI_ACTION);
                 return true;
             default:;
             }
         }
    }
        return false;
    
}


void GuiDropDownList::setRect(const Rect2D& rect) {
     m_rect = rect;
     m_clickRect = theme()->dropDownListToClickBounds(rect, m_captionWidth);
}


const GuiText& GuiDropDownList::selectedValue() const {
    if (m_usePrefixTreeMenus){
        return m_menu->selectedValue();
    } else {
        if (m_listValue.size() > 0) {
            return m_listValue[selectedIndex()];
        } else {
            const static GuiText empty;
            return empty;
        }
    }
}


void GuiDropDownList::setList(const Array<GuiText>& c) {
    if (m_usePrefixTreeMenus){
        m_menu->setList(c);
    } else {
        m_listValue = c;
        *m_indexValue = iClamp(*m_indexValue, 0, m_listValue.size() - 1);
        m_menu.reset();
    }
}


void GuiDropDownList::setList(const Array<String>& c) {
    if (m_usePrefixTreeMenus){
        m_menu->setList(c);
    } else {
        m_listValue.resize(c.size());
        for (int i = 0; i < c.size(); ++i) {
            m_listValue[i] = c[i];
        }
        *m_indexValue = selectedIndex();
        m_menu.reset();
    }
}


void GuiDropDownList::clear() {
    if (m_usePrefixTreeMenus){
        m_listValue.clear();
        m_menu->clear();
    } else {
        m_listValue.clear();
        *m_indexValue = 0;
        m_menu.reset();
    }
}


void GuiDropDownList::append(const GuiText& c) {
    if (m_usePrefixTreeMenus){
        m_listValue.append(c);
        m_menu = nullptr;
        menu();
    } else {
        m_listValue.append(c);
        m_menu.reset();
    }
}

}
