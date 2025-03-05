/**
  \file G3D-app.lib/include/G3D-app/GuiMenu.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_GuiMenu
#include "G3D-base/Pointer.h"
#include "G3D-base/Array.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiControl.h"

namespace G3D {

class PrefixTree;
class GuiPane;
class GuiScrollPane;

/**A special "popup" window that hides itself when it loses focus.
   Used by GuiDropDownList for the popup and can be used to build context menus. */
class GuiMenu : public GuiWindow {
friend class GuiDropDownList;
protected:

    GuiControl::Callback            m_actionCallback;
    GuiControl*                     m_eventSource;

    Array<String>*                  m_stringListValue;
    Array<GuiText>*                 m_captionListValue;

    /** The created labels for each menu item */
    Array<GuiControl*>              m_labelArray;
    Pointer<int>                    m_indexValue;

    /** Which of the two list values to use */
    bool                            m_useStringList;

    /** Window to select when the menu is closed */
    GuiWindow*                      m_superior;

    /** Scroll pane to stick the menu options in if there are too many. Null if no need for scroll */
    GuiScrollPane*                  m_innerScrollPane;

    /** Mouse is over this option */
    int                             m_highlightIndex;

    bool                            m_usePrefixTreeMenus;

    //Prefix tree section, member variables

    /** The PrefixTree associated with this menu*/
    shared_ptr<PrefixTree>          m_prefixTree;

    /** The text selected by the menu */
    GuiText                         m_selectedValue;

    /** The outward facing GuiMenu that needs to return final selection*/
    shared_ptr<GuiMenu>             m_prefixRootMenu;

    /** Within child Menus, which index was selected*/
    int                             m_prefixIndex;

    /** Used to quickly build up child menus on selection of an index*/
    Array<shared_ptr<PrefixTree>>   m_prefixNodes;

    //Prefix tree section, helper functions

    /** If the menu contains s, then the value is set to s*/
    void setSelectedValue(const String& s);

    /** Helper function for prefix menus to put menu at menuPosition*/
    void showPrefixMenu(const Vector2& menuPosition);
    
    /** Links new menu and current menu as child-parent menu pair */
    void appendMenu(const shared_ptr<GuiMenu>& menu);
    
    /** Returns true if s is one of the values in the current menu*/
    bool containsValue(const String& s) const;
    
    /** Returns the selected Value of the menu */
    GuiText& selectedValue();
    
    /** Sets the menu to have the values in c*/
    void setList(const Array<GuiText>& c); 
    
    /** Sets the menu to have the values in c*/
    void setList(const Array<String>& c);
    
    /** Resets the menu by deleting the current prefixTree */
    void clear();
    
    /** Hides the current menu and then sets any child menu to be null */
    void closePrefixMenu();
    
    /** evaluates onClick events that happen on prefix menus */
    bool prefixClickedOn(const Point2 click);
    
    /** evaluates the action to be taken when an index in a prefix menu is selected*/
    void prefixIndexSelected(int i);
    
    /** returns a prefix menu for specific prefixTree node*/
    shared_ptr<GuiMenu> createPrefixMenu(const shared_ptr<PrefixTree>& node, const Pointer<int>& selectedIndex, const shared_ptr<GuiTheme>& theme);
    
    /** Returns the selected prefix tree */
    const shared_ptr<PrefixTree>& selectedNode(int index);
    
    /** Finds the absolute index of a selected root in a prefix menu structure*/
    int findAbsoluteIndex();
    
    /** Helper function to pass on state from parent to child prefix menus */
    void initializeChildMenu(WidgetManager* manager, GuiWindow* superior, GuiControl* eventSource, GuiControl::Callback actionCallback,const shared_ptr<GuiMenu>& prefixRootMenu);
    
    /** Builds the root node of a prefix Menu, which must be initialized differently then the child nodes */
    shared_ptr<GuiMenu> buildRootPrefixMenu(const shared_ptr<GuiTheme> theme, Array<GuiText>* listValue, const Pointer<int>& indexValue);
    
    /** Builds the root node of a prefix Menu, which must be initialized differently then the child nodes */
    shared_ptr<GuiMenu> buildRootPrefixMenu(const shared_ptr<GuiTheme> theme, Array<String>* listValue, const Pointer<int>& indexValue);


    //end of prefix tree section

    GuiMenu(const shared_ptr<GuiTheme>& theme, const Rect2D& rect, Array<GuiText>* listPtr, const Pointer<int>& indexValue, bool usePrefixTreeMenus);
    GuiMenu(const shared_ptr<GuiTheme>& theme, const Rect2D& rect, Array<String>* listPtr, const Pointer<int>& indexValue, bool usePrefixTreeMenus);

    /** Returns -1 if none */
    int labelIndexUnderMouse(Vector2 click) const;

    /** Fires an action event */
    void fireMyEvent(GEventType type);

    void init(const shared_ptr<GuiTheme>& skin, const Rect2D& rect, const Array<GuiText>& listPtr, const Pointer<int>& indexValue);

    /** Called from render to draw chevrons and highlighting before child content. */
    virtual void renderDecorations(RenderDevice* rd) const;

    void showInternal(WidgetManager* manager, GuiWindow* superior, GuiControl* eventSource, const Vector2& position, bool modal, const GuiControl::Callback& actionCallback);

public:

    /** A submenu. If the child is not null, then this window will not close. */
    shared_ptr<GuiMenu>             m_child;
    shared_ptr<GuiMenu>             m_parent;

    static shared_ptr<GuiMenu> create(const shared_ptr<GuiTheme>& theme, Array<GuiText>* listPtr, const Pointer<int>& indexValue, bool usePrefixTreeMenus = false, bool root = false);
    
    static shared_ptr<GuiMenu> create(const shared_ptr<GuiTheme>& theme, Array<String>* listPtr, const Pointer<int>& indexValue, bool usePrefixTreeMenus = false, bool root = false);

    virtual bool onEvent(const GEvent& event) override;

    virtual void render(RenderDevice* rd) const override;
   
    void hide();

    const Rect2D& labelRect(int i) { return m_labelArray[i]->clickRect(); }

    /** \param superior The window from which the menu is being created. */
    virtual void show(WidgetManager* manager, GuiWindow* superior, GuiControl* eventSource, const Vector2& position, bool modal = false, const GuiControl::Callback& actionCallback = GuiControl::Callback());
};

}
