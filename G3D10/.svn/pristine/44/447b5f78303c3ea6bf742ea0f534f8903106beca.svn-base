/**
  \file G3D-app.lib/source/GuiMenu.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-base/PrefixTree.h"
#include "G3D-app/GuiMenu.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiPane.h"
#include "G3D-gfx/RenderDevice.h"


namespace G3D {

shared_ptr<GuiMenu> GuiMenu::create(const shared_ptr<GuiTheme>& theme, Array<String>* listPtr, const Pointer<int>& indexValue, bool usePrefixTreeMenus, bool root) {
    if (root){
        const shared_ptr<GuiMenu>& menu = createShared<GuiMenu>(theme, Rect2D::xywh(0, 0, 120, 0), listPtr, indexValue, usePrefixTreeMenus);
        return menu->buildRootPrefixMenu(theme, listPtr, indexValue);
    } else {
        return createShared<GuiMenu>(theme, Rect2D::xywh(0, 0, 120, 0), listPtr, indexValue, usePrefixTreeMenus);
    }
}
    

shared_ptr<GuiMenu> GuiMenu::create(const shared_ptr<GuiTheme>& theme, Array<GuiText>* listPtr, const Pointer<int>& indexValue, bool usePrefixTreeMenus, bool root) {
    if (root){
        const shared_ptr<GuiMenu>& menu = createShared<GuiMenu>(theme, Rect2D::xywh(0, 0, 120, 0), listPtr, indexValue, usePrefixTreeMenus);
        return menu->buildRootPrefixMenu(theme, listPtr, indexValue);
    } else {
        return createShared<GuiMenu>(theme, Rect2D::xywh(0, 0, 120, 0), listPtr, indexValue, usePrefixTreeMenus);
    }
}
    

static int menuHeight(int numLabels) {
    return (numLabels * GuiPane::CONTROL_HEIGHT) + GuiPane::CONTROL_PADDING;
}


GuiMenu::GuiMenu(const shared_ptr<GuiTheme>& skin, const Rect2D& rect, Array<String>* listPtr, const Pointer<int>& indexValue, bool usePrefixTreeMenus) : 
    GuiWindow("", skin, rect, GuiTheme::MENU_WINDOW_STYLE, NO_CLOSE), 
    m_eventSource(nullptr), 
    m_stringListValue(listPtr), 
    m_captionListValue(nullptr), 
    m_indexValue(indexValue), 
    m_useStringList(true),
    m_superior(nullptr),
    m_innerScrollPane(nullptr), 
    m_usePrefixTreeMenus(usePrefixTreeMenus),
    m_child(nullptr) {

    Array<GuiText> guitextList;
    guitextList.resize(listPtr->size());
    for (int i = 0; i < guitextList.size(); ++i) {
        guitextList[i] = (*listPtr)[i];
    }

    init(skin, rect, guitextList, indexValue);
}


GuiMenu::GuiMenu(const shared_ptr<GuiTheme>& skin, const Rect2D& rect, Array<GuiText>* listPtr, 
                 const Pointer<int>& indexValue, bool usePrefixTreeMenus) : 
    GuiWindow("", skin, rect, GuiTheme::MENU_WINDOW_STYLE, NO_CLOSE),
    m_stringListValue(nullptr), 
    m_captionListValue(listPtr),
    m_indexValue(indexValue),
    m_useStringList(false),
    m_superior(nullptr),
    m_innerScrollPane(nullptr),
    m_usePrefixTreeMenus(usePrefixTreeMenus),
    m_child(nullptr) {

    init(skin, rect, *listPtr, indexValue);
}


void GuiMenu::init(const shared_ptr<GuiTheme>& skin, const Rect2D& rect, const Array<GuiText>& listPtr, const Pointer<int>& indexValue) {
    GuiPane* innerPane;
    const int windowHeight = RenderDevice::current->window()->height();

    m_innerScrollPane = pane()->addScrollPane(true, false, GuiTheme::BORDERLESS_SCROLL_PANE_STYLE);
    m_innerScrollPane->setPosition(0, 0);
    m_innerScrollPane->setHeight(float(min(windowHeight, menuHeight(listPtr.size())) + GuiPane::CONTROL_PADDING));
    innerPane = m_innerScrollPane->viewPane();
    
    innerPane->setHeight(0);
    m_labelArray.resize(listPtr.size());

    for (int i = 0; i < listPtr.size(); ++i) {
        m_labelArray[i] = innerPane->addLabel(listPtr[i]);
    }

    if (notNull(m_innerScrollPane)) {
        m_innerScrollPane->pack();
    }

    pack();
    m_highlightIndex = *m_indexValue;
}


bool GuiMenu::onEvent(const GEvent& event) {
    if (! m_visible) {
        return false;
    }

    // Hide all menus on escape key
    if ((event.type == GEventType::KEY_DOWN) && 
        (event.key.keysym.sym == GKey::ESCAPE)) {        
        fireMyEvent(GEventType::GUI_CANCEL);
        hide();
        return true;
    }

    if ((event.type == GEventType::KEY_DOWN) &&
        (event.key.keysym.sym == GKey::UP)) {
        if (m_highlightIndex > 0) {
            --m_highlightIndex;
        } else {
            m_highlightIndex = m_labelArray.size() - 1;
        }
        *m_indexValue = m_highlightIndex;
        return true;
    }

    if ((event.type == GEventType::KEY_DOWN) &&
        (event.key.keysym.sym == GKey::DOWN)) {
        if (m_highlightIndex < m_labelArray.size() - 1) {
            ++m_highlightIndex;
        } else {
            m_highlightIndex = 0;
        }
        *m_indexValue = m_highlightIndex;
        return true;
    }

    if ((event.type == GEventType::KEY_DOWN) &&
        (event.key.keysym.sym == GKey::RETURN)) {
        if(m_usePrefixTreeMenus){
            prefixIndexSelected(m_highlightIndex);
        } else {
            *m_indexValue = m_highlightIndex;
            m_actionCallback.execute();
            fireMyEvent(GEventType::GUI_ACTION);
        }
        return true;
    }

    if ((event.type == GEventType::GUI_ACTION) &&
        (event.gui.control == m_eventSource) &&
        isNull(m_child)) { 
        
        if (! m_usePrefixTreeMenus) {
            hide();
        }

        m_actionCallback.execute();
        // Do not consume on a callback
        return false;
    }
    
    if (event.type == GEventType::MOUSE_BUTTON_DOWN) {
        // See what was clicked on
        const Point2 click = Point2(event.button.x, event.button.y) / m_pixelScale;
        if (m_usePrefixTreeMenus){
            
            if (m_clientRect.contains(click)) {
                if (! isNull(m_child)){
                    m_child->hide();
                }

                int i = labelIndexUnderMouse(click);
                if (i >= 0) {
                    prefixIndexSelected(i);
                    return true;
                }
                //go to scollbar?
            } else {
                if(!isNull(m_parent)){
                    return m_parent->prefixClickedOn(click);
                } else {
                    m_prefixRootMenu->closePrefixMenu();
                    return false;
                }
            }

        } else if (m_clientRect.contains(click)) {
            const int i = labelIndexUnderMouse(click);
            if (i >= 0) {
                // There was a valid label index. Invoke 
                // the menu's callback and then fire the action.
                *m_indexValue = i;
                fireMyEvent(GEventType::GUI_ACTION);

                // Do NOT hide the window in the case where we fire the ACTION event...if we do, then it can't
                // receive its own action callback event

                return true;
            }
            // (The click may have gone to the scroll bar in this case)    
        } else {
            // Clicked off the menu
            hide();
            return false;
        }
    } else if (event.type == GEventType::MOUSE_MOTION) {
        const Point2 position = Point2(event.motion.x, event.motion.y) / m_pixelScale;

        // If the mouse is not in the inner scroll pane, it might be on the scroll bar, so event propagation should continue
        if (m_innerScrollPane->viewPane()->clientRect().contains(position - m_clientRect.x0y0())) {
            int index = labelIndexUnderMouse(position);
            if ((index != m_prefixIndex) && (index >= 0)) {
                m_highlightIndex = index;
                m_prefixIndex = -1;
                if (notNull(m_child)) {
                    m_child->hide();
                }
            }
            // Don't let any other window see this event
            return true;
        }
    } 

    const bool handled = GuiWindow::onEvent(event);

    if (! (focused() || (notNull(m_innerScrollPane) && m_innerScrollPane->enabled())) && 
        notNull(m_child)) {
        hide();
    }

    return handled;
}


int GuiMenu::labelIndexUnderMouse(Vector2 click) const {
        
    click += pane()->clientRect().x0y0() - m_clientRect.x0y0();
    
    float width = pane()->clientRect().width();
    if (notNull(m_innerScrollPane)) {
        click += m_innerScrollPane->paneOffset();
        width = m_innerScrollPane->viewPane()->rect().width();
    }

    for (int i = 0; i < m_labelArray.size(); ++i) {
        // Extend the rect to the full width of the menu
        Rect2D rect = m_labelArray[i]->rect();
        rect = Rect2D::xywh(rect.x0(), rect.y0(), width, rect.height());
        if (rect.contains(click)) {
            return i;
        }
    }

    // Nothing found
    return -1;    
}


void GuiMenu::fireMyEvent(GEventType type) {
    GEvent e;
    e.gui.type = type;
    e.gui.control = m_eventSource;
    Widget::fireEvent(e);
}


void GuiMenu::show(WidgetManager* manager, GuiWindow* superior, GuiControl* eventSource, const Vector2& position, bool modal, const GuiControl::Callback& actionCallback) {
    if (m_usePrefixTreeMenus){
        initializeChildMenu(manager, superior, eventSource, actionCallback, dynamic_pointer_cast<GuiMenu>(shared_from_this()));
        showPrefixMenu(position);
    } else {
        showInternal(manager, superior, eventSource, position, modal, actionCallback);
    }
}


void GuiMenu::showInternal(WidgetManager* manager, GuiWindow* superior, GuiControl* eventSource, const Vector2& position, bool modal, const GuiControl::Callback& actionCallback) {
    m_actionCallback = actionCallback;
    m_superior = superior;
    debugAssertM(notNull(eventSource), "Event source may not be nullptr");
    m_eventSource = eventSource;
    manager->add(dynamic_pointer_cast<Widget>(shared_from_this()));

    // Clamp position to screen bounds
    OSWindow* osWindow = notNull(superior) ? superior->window() : RenderDevice::current->window();
    const Vector2 high(osWindow->width() - m_rect.width(), osWindow->height() - m_rect.height());
    const Vector2 actualPos = position.min(high).max(Vector2(0, 0));

    moveTo(actualPos);
    manager->setFocusedWidget(dynamic_pointer_cast<Widget>(shared_from_this()));

    if (modal) {
        showModal(dynamic_pointer_cast<GuiWindow>(superior->shared_from_this()));
    } else {
        setVisible(true);
    }
}


void GuiMenu::hide() {
    if (isNull(m_manager)) return; // TODO remove
    debugAssert(notNull(m_manager));

    // First, recursively hide and remove all children
    if (m_child) {
        m_child->hide();
    }

    // Make this menu disappear and return focus to superior window
    setVisible(false);
    const shared_ptr<Widget>& widget = dynamic_pointer_cast<Widget>(shared_from_this());

    if (notNull(widget) && m_manager->contains(widget)) {
        m_manager->remove(widget);
    }

    // Detach this child from its parent
    m_superior = nullptr;
    if (notNull(m_parent)) {
        m_parent->m_child = nullptr;
        m_parent = nullptr;
    }
}


void GuiMenu::render(RenderDevice* rd) const {
    if (m_morph.active) {
        GuiMenu* me = const_cast<GuiMenu*>(this);
        me->m_morph.update(me);
    }
    
    m_theme->beginRendering(rd, m_pixelScale);
    {
        m_theme->renderWindow(m_rect, focused(), false, false, false, m_text, GuiTheme::WindowStyle(m_style));
        m_theme->pushClientRect(m_clientRect); {
            renderDecorations(rd);            
            m_rootPane->render(rd, m_theme, m_enabled);
        } m_theme->popClientRect();
    }
    m_theme->endRendering();
}


void GuiMenu::renderDecorations(RenderDevice* rd) const {
    // Draw the highlight (the root pane's background is invisible, so it will not overwrite this rectangle)
    if ((m_highlightIndex >= 0) && (m_highlightIndex < m_labelArray.size())) {
        const Rect2D& r = m_labelArray[m_highlightIndex]->rect();
        const float verticalOffset = notNull(m_innerScrollPane) ? -m_innerScrollPane->verticalOffset() : 0.0f;
        m_theme->renderSelection(Rect2D::xywh(0, r.y0() + verticalOffset, m_clientRect.width(), r.height()));
    }
}


void GuiMenu::showPrefixMenu(const Vector2& menuPosition) {
    showInternal(
        m_manager,
        m_superior,
        m_eventSource,
        menuPosition,
        false,
        m_actionCallback);
} 


void GuiMenu::initializeChildMenu(WidgetManager* manager, GuiWindow* superior, GuiControl* eventSource, GuiControl::Callback actionCallback, const shared_ptr<GuiMenu>& prefixRootMenu){
    m_manager = manager;
    m_superior = superior;
    m_eventSource = eventSource;
    m_actionCallback = actionCallback;
    m_prefixRootMenu = prefixRootMenu;
    m_prefixIndex = -1;
}


void GuiMenu::appendMenu(const shared_ptr<GuiMenu>& menu) {
    m_child = menu;
    menu->m_parent = dynamic_pointer_cast<GuiMenu>(shared_from_this());
}


void GuiMenu::setSelectedValue(const String& s){
     if (containsValue(s)) {
        m_selectedValue = s;
    }
}


bool GuiMenu::containsValue(const String& s) const{
     if (isNull(m_prefixTree)){
        return false;
     }
     return m_prefixTree->contains(s);
}


GuiText& GuiMenu::selectedValue() {
    return m_selectedValue;
}


void GuiMenu::setList(const Array<GuiText>& c) {
    clear();
    m_prefixTree = PrefixTree::create(c, true);
}


void GuiMenu::setList(const Array<String>& c) {
    clear();
    m_prefixTree = PrefixTree::create(c, true);
}


void GuiMenu::clear(){
    m_prefixTree = nullptr;
}


void GuiMenu::closePrefixMenu(){
    hide();
    m_child = nullptr;
}


bool GuiMenu::prefixClickedOn(const Point2 click){
    if (m_clientRect.contains(click)) {
        if (! isNull(m_child)) {
            m_child->hide();
        }
        int i = labelIndexUnderMouse(click);
        if (i >= 0) {
            prefixIndexSelected(i);
            return true;
        }
        return false;
    } else {
        if (!isNull(m_parent)) {
            return m_parent->prefixClickedOn(click);
        } else {
            m_prefixRootMenu->closePrefixMenu();
            return false;
        }
    }
}


int GuiMenu::findAbsoluteIndex(){
    if (isNull(m_child)){
        return 0;
    }

    int totalNodes = 0;
    for (int i = 0; i < m_prefixIndex; ++i) {
        if (selectedNode(i)->isLeaf()){
            ++totalNodes;
        } else {
            totalNodes += selectedNode(i)->size();
        }
    }

    return totalNodes + m_child->findAbsoluteIndex();
}


void GuiMenu::prefixIndexSelected(int i) {
    const shared_ptr<PrefixTree>& node = selectedNode(i);
    const shared_ptr<GuiMenu>& menu = createPrefixMenu(node, &m_prefixIndex, theme());
    appendMenu(menu);   
    
    m_prefixIndex = i;
    
    if (node->isLeaf()) {
        // Set the selectedValue and indexValue of the outward facing guimenu
        m_prefixRootMenu->m_selectedValue = node->value();
        *m_prefixRootMenu->m_indexValue = m_prefixRootMenu->findAbsoluteIndex();
        
        // Execute our callback and the close the menu
        m_actionCallback.execute();
        m_prefixRootMenu->closePrefixMenu();
    } else {
        const Rect2D& parentRect = labelRect(m_prefixIndex);
        // Offset to world space
        const Point2& menuPosition = Rect2D::xywh(rect().x0() + 2, parentRect.y0() + rect().y0(), rect().width(), parentRect.height()).x1y0();
        menu->initializeChildMenu(m_manager,m_superior,m_eventSource,m_actionCallback,m_prefixRootMenu);
        menu->m_prefixTree = node;
        menu->showInternal(m_manager,m_superior,m_eventSource,menuPosition,false,m_actionCallback);
    }
}


shared_ptr<GuiMenu> GuiMenu::buildRootPrefixMenu(const shared_ptr<GuiTheme> theme, Array<GuiText>* listValue, const Pointer<int>& indexValue){
    const shared_ptr<PrefixTree>& prefixTree = PrefixTree::create(*listValue,true);
    shared_ptr<GuiMenu> menu = createPrefixMenu(prefixTree, indexValue, theme);
    menu->m_selectedValue = listValue->size() > 0 ? listValue->first() : "";
    menu->m_prefixTree  = prefixTree; 
    menu->m_captionListValue = listValue;
    return menu;
}


shared_ptr<GuiMenu> GuiMenu::buildRootPrefixMenu(const shared_ptr<GuiTheme> theme,Array<String>* listValue, const Pointer<int>& indexValue){
    shared_ptr<PrefixTree> prefixTree = PrefixTree::create(*listValue,true);
    shared_ptr<GuiMenu> menu = createPrefixMenu(prefixTree, indexValue, theme);
    menu->m_selectedValue = listValue->size() > 0 ? listValue->first() : "";
    menu->m_prefixTree  = prefixTree; 
    menu->m_stringListValue = listValue;
    return menu;
}


shared_ptr<GuiMenu> GuiMenu::createPrefixMenu(const shared_ptr<PrefixTree>& node, const Pointer<int>& selectedIndex, const shared_ptr<GuiTheme>& theme){
    debugAssert(notNull(node));
    Array<String> m_items;
    Array<shared_ptr<PrefixTree>> prefixNodes;

    for (const shared_ptr<PrefixTree>& child : node->children()) {
        shared_ptr<PrefixTree> branchPoint;
        const String& menuItemName = child->getPathToBranch(branchPoint);
        if (! menuItemName.empty()){
            m_items.append(menuItemName);
            prefixNodes.append(branchPoint);
        } else{
            // instead of blank spaces, these should be (default)
            m_items.append(" (Default)");
            prefixNodes.append(branchPoint);
        }
    }

    const shared_ptr<GuiMenu>& menu = create(theme, &m_items, selectedIndex, true);
    menu->m_prefixNodes = prefixNodes;
    return menu;
}


const shared_ptr<PrefixTree>& GuiMenu::selectedNode(int index) {
   return m_prefixNodes[index];
}

}
