/**
  \file G3D-app.lib/source/UserInput.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/UserInput.h"
#include "G3D-gfx/RenderDevice.h"

namespace G3D {


UserInput::UserInput
(OSWindow*                          window) {
    init(window, nullptr);
}


void UserInput::init
(OSWindow*                    window,
 Table<GKey, UIFunction>*     keyMapping) {

    _pureDeltaMouse = false;
    deltaMouse = Vector2(0, 0);

    alwaysAssertM(notNull(window), "Window must not be nullptr");

    _window = window;
    windowCenter = Vector2((float)window->width() / 2.0f, (float)window->height() / 2.0f);

    up = left = down = right = false;
    jx = jy = 0.0;

    inEventProcessing = false;

    bool tempMapping = isNull(keyMapping);

    if (tempMapping) {
        keyMapping = new Table<GKey, UIFunction>();
        keyMapping->set(GKey::RIGHT, RIGHT);
        keyMapping->set(GKey::LEFT, LEFT);
        keyMapping->set(GKey::UP, UP);
        keyMapping->set(GKey::DOWN, DOWN);
        keyMapping->set(GKey('d'), RIGHT);
        keyMapping->set(GKey('a'), LEFT);
        keyMapping->set(GKey('w'), UP);
        keyMapping->set(GKey('s'), DOWN);
    }

    // Will be initialized by setKeyMapping don't need to memset
    keyState.resize(GKey::LAST);
    
    keyFunction.resize(keyState.size());
    setKeyMapping(keyMapping);

    if (tempMapping) {
        delete keyMapping;
        keyMapping = nullptr;
    }

    useJoystick = (_window->numJoysticks() > 0);
    _window->getRelativeMouseState(mouse, mouseButtons);
    guiMouse = mouse;

    appHadFocus = _window->hasFocus();
}


OSWindow* UserInput::window() const {
    return _window;
}


void UserInput::setKeyMapping
(Table<GKey, UIFunction>* keyMapping) {

    for (int i = (keyState.size() - 1); i >= 0; --i) {
        keyState[(int)i]    = false;
        if ((i < 'A' || i > 'Z') && keyMapping->containsKey(GKey(i))) {
            keyFunction[(int)i] = keyMapping->get(GKey(i));
        } else {
            keyFunction[(int)i] = NONE;
        }
    }
}


UserInput::~UserInput() {
}


void UserInput::processEvent(const GEvent& event) {
    
    debugAssert(inEventProcessing);
    // Translate everything into a key code then call processKey

    switch(event.type) {
    case GEventType::KEY_UP:
        processKey(event.key.keysym.sym, GEventType::KEY_UP);
        break;
        
    case GEventType::KEY_DOWN:
        processKey(event.key.keysym.sym, GEventType::KEY_DOWN);
        break;
        
    case GEventType::MOUSE_BUTTON_DOWN:
        processKey((GKey)(GKey::LEFT_MOUSE + event.button.button), GEventType::KEY_DOWN);
        break;
        
    case GEventType::MOUSE_BUTTON_UP:
        processKey(GKey(GKey::LEFT_MOUSE + event.button.button), GEventType::KEY_UP);
        break;
    }
}


void UserInput::beginEvents() {
    debugAssert(! inEventProcessing);
    inEventProcessing = true;
    justPressed.resize(0, DONT_SHRINK_UNDERLYING_ARRAY);
    justReleased.resize(0, DONT_SHRINK_UNDERLYING_ARRAY);
}


// Different operating system drivers seem to map the Xbox360 controller differently
// This has to match GameController.cpp
#ifdef G3D_WINDOWS
    enum { LEFT_X_AXIS = 0, LEFT_Y_AXIS = 1, RIGHT_X_AXIS = 2, RIGHT_Y_AXIS = 3,  TRIGGER_LEFT_AXIS = 4, TRIGGER_RIGHT_AXIS = 5, NUM_AXES = 6};
    static const int buttonRemap[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
    static const float MAYBE_INVERT_LEFT_Y  = -1.0f;
    static const float MAYBE_INVERT_RIGHT_Y = 1.0f;
    static const float MAYBE_INVERT_TRIGGER_Y = 1.0f;
# else
    // Currently set based on OSX driver, which has inverted axes
    enum { RIGHT_X_AXIS = 2, RIGHT_Y_AXIS = 3, LEFT_X_AXIS = 0, LEFT_Y_AXIS = 1, TRIGGER_LEFT_AXIS = 4, TRIGGER_RIGHT_AXIS = 5};
    static const int buttonRemap[] = {11, 12, 13, 14, 8, 9, 5, 4, 6, 7, 0, 3, 1, 2, 10};
    static const float MAYBE_INVERT_LEFT_Y  = -1.0f;
    static const float MAYBE_INVERT_RIGHT_Y = 1.0f;
    static const float MAYBE_INVERT_TRIGGER_Y = 1.0f;
#endif

void UserInput::endEvents() {
    debugAssert(inEventProcessing);

    inEventProcessing = false;
    if (useJoystick) {
        _window->getJoystickState(0, axis, button);

        if (axis.size() >= 5) {
            static const float threshold = 0.15f;
            jx = axis[LEFT_X_AXIS];
            if (abs(jx) < threshold) { jx = 0; }

            jy = axis[LEFT_Y_AXIS] * MAYBE_INVERT_LEFT_Y;
            if (abs(jy) < threshold) { jy = 0; }

            jx = square(jx) * sign(jx);
            jy = square(jy) * sign(jy);

            m_leftStick = Vector2((float)jx, (float)jy);
         
            m_rightStick.x = axis[RIGHT_X_AXIS];
            if (abs(m_rightStick.x) < threshold) { m_rightStick.x = 0; }

            m_rightStick.y = axis[RIGHT_Y_AXIS] * MAYBE_INVERT_RIGHT_Y;
            if (abs(m_rightStick.y) < threshold) { m_rightStick.y = 0; }

            m_rightStick.x = square(m_rightStick.x) * sign(m_rightStick.x);
            m_rightStick.y = square(m_rightStick.y) * sign(m_rightStick.y);

            if (axis.length() > 5) { 
                // On Windows under DI8 the triggers are mapped to the same axis
                // and are not really useful.  You have to use XInput to distinguish them.

                m_triggers.x = axis[TRIGGER_LEFT_AXIS];
                if (abs(m_triggers.x) < threshold) { m_triggers.x = 0; }

                // Intentionally negated
                m_triggers.y = -axis[TRIGGER_RIGHT_AXIS] * MAYBE_INVERT_TRIGGER_Y;
                if (abs(m_triggers.y) < threshold) { m_triggers.y = 0; }

                m_triggers.x = square(m_triggers.x) * sign(m_triggers.x);
                m_triggers.y = square(m_triggers.y) * sign(m_triggers.y);
            }
        }

        // Set up fake button keys
        for (int b = 0; b < min(button.size(), 15); ++b) {
            const int buttonIndex = buttonRemap[b];

            if (! keyState[GKey(GKey::CONTROLLER_A + b)] && button[buttonIndex]) {
                justPressed.append(GKey(GKey::CONTROLLER_A + b));
            } else if (keyState[GKey(GKey::CONTROLLER_A + b)] && !button[buttonIndex]) {
                justReleased.append(GKey(GKey::CONTROLLER_A + b));
            }

            keyState[GKey(GKey::CONTROLLER_A + b)] = button[buttonIndex];
        }
    }

    windowCenter = Vector2((float)window()->width(), (float)window()->height()) / 2.0f;

    Vector2 oldMouse = mouse;
    _window->getRelativeMouseState(mouse, mouseButtons);

    if (abs(mouse.x) > 100000) {
        // Sometimes we get bad values on the first frame;
        // ignore them.
        mouse = oldMouse;
    }

    deltaMouse = mouse - oldMouse;

    bool focus = _window->hasFocus();

    if (_pureDeltaMouse) {

        // Reset the mouse periodically.  We can't do this every
        // frame or the mouse will not be able to move substantially
        // at high frame rates.
        if ((mouse.x < windowCenter.x * 0.5f) || (mouse.x > windowCenter.x * 1.5f) ||
            (mouse.y < windowCenter.y * 0.5f) || (mouse.y > windowCenter.y * 1.5f)) {
        
            mouse = windowCenter;
            if (focus) {
                setMouseXY(mouse);
            }
        }

        // Handle focus-in and focus-out gracefully
        if (focus && ! appHadFocus) {
            // We just gained focus.
            grabMouse();
        } else if (! focus && appHadFocus) {
            // Just lost focus
            releaseMouse();
        }
    } else {
        guiMouse = mouse;
    }

    appHadFocus = focus;
}


Vector2 UserInput::getXY() const {
    return Vector2(getX(), getY());
}


Vector2 UserInput::virtualStick1() const {
    Vector2 s = m_leftStick;

    // Keyboard may override joystick
    if (left && ! right) {
        s.x = -1.0f;
    } else if (right && ! left) {
        s.x = 1.0f;
    }

    if (down && ! up) {
        s.y = -1.0f;
    } else if (up && ! down) {
        s.y = 1.0f;
    }

    return s;
}


Vector2 UserInput::virtualStick2() const {
    Vector2 s = m_rightStick;

    const bool rotLeft  = keyState['Q'];
    const bool rotRight = keyState['E'];

    if (rotLeft && ! rotRight) {
        s.x = -1.0f;
    } else if (rotRight && ! rotLeft) {
        s.x = 1.0f;
    }

    // There are no default up/down keys



    // Distance the mouse must move to translate to a joystick at max rotation.  Only use
    // the mouse if it is not currently visible
    static const float MOUSE_MAX_PIXELS = 120.0f;
    const Vector2& m = (notNull(_window) && (_window->mouseHideCount() >= 1)) ? clamp(deltaMouse / MOUSE_MAX_PIXELS, Vector2(-1, -1), Vector2(1, 1)) : Vector2::zero();

    // If the mouse has higher magnitude than the keyboard or joystick
    // on either axis, override that axis
    return s.maxAbs(m);
}


Vector2 UserInput::virtualStick3() const {
    return m_triggers;
}


float UserInput::getX() const {

    if (left && !right) {
        return -1.0f;
    } else if (right && !left) {
        return 1.0f;
    }
    
    if (useJoystick && (fabs(m_leftStick.x) > 0.05f)) {
        return m_leftStick.x;
    }
    
    return 0.0f;
}


float UserInput::getY() const {
    if (down && !up) {
        return -1.0f;
    } else if (up && !down) {
        return 1.0f;
    }
    
    if (useJoystick && (fabs(m_leftStick.y) > 0.05f)) {
        return m_leftStick.y;
    }
    
    return 0.0f;
}


void UserInput::processKey(GKey code, int event) {
    bool state = (event == GEventType::KEY_DOWN);

    if ((code < GKey(keyFunction.size()) && (code >= GKey(0)))) {
        switch (keyFunction[(int)code]) {
        case RIGHT:
            right = state;
            break;

        case LEFT:
            left = state;
            break;

        case UP:
            up = state;
            break;

        case DOWN:
            down = state;
            break;

        case NONE:
            break;
        }

        keyState[(int)code] = state;

        if (state) {
            justPressed.append(code);
        } else {
            justReleased.append(code);
        }
    }
}


float UserInput::mouseDX() const {
    return deltaMouse.x;
}


float UserInput::mouseDY() const {
    return deltaMouse.y;
}


Vector2 UserInput::mouseDXY() const {
    return deltaMouse;
}


void UserInput::setMouseXY(float x, float y) {
    mouse.x = x;
    mouse.y = y;
    _window->setRelativeMousePosition(mouse);
}


int UserInput::getNumJoysticks() const {
    return _window->numJoysticks();
}


bool UserInput::keyDown(GKey code) const {
    debugAssertM((int(code) < 'A') || (int(code) > 'Z'), "Use lower-case letters for alphabetic key codes");
    if (code > GKey::LAST) {
        return false;
    } else {
        return keyState[(int)code];
    }
}


bool UserInput::keyPressed(GKey code) const {
    debugAssertM((int(code) < 'A') || (int(code) > 'Z'), "Use lower-case letters for alphabetic key codes");
    for (int i = justPressed.size() - 1; i >= 0; --i) {
        if (code == justPressed[i]) {
            return true;
        }
    }

    return false;
}


bool UserInput::keyReleased(GKey code) const {
    debugAssertM((int(code) < 'A') || (int(code) > 'Z'), "Use lower-case letters for alphabetic key codes");
    for (int i = justReleased.size() - 1; i >= 0; --i) {
        if (code == justReleased[i]) {
            return true;
        }
    }

    return false;
}


void UserInput::pressedKeys(Array<GKey>& code) const {
    code.resize(justPressed.size());
    memcpy(code.getCArray(), justPressed.getCArray(), sizeof(GKey) * justPressed.size());
}


void UserInput::releasedKeys(Array<GKey>& code) const {
    code.resize(justReleased.size());
    memcpy(code.getCArray(), justReleased.getCArray(), sizeof(GKey) * justReleased.size());
}


bool UserInput::anyKeyPressed() const {
    return (justPressed.size() > 0);
}


bool UserInput::pureDeltaMouse() const {
    return _pureDeltaMouse;
}


void UserInput::grabMouse() {
    uint8 dummy;
    // Save the old mouse position for when we deactivate
    _window->getRelativeMouseState(guiMouse, dummy);
    
    mouse = windowCenter;
    _window->setRelativeMousePosition(mouse);
    deltaMouse = Vector2(0, 0);
 
    window()->incMouseHideCount();

    #ifndef G3D_DEBUG
        // In debug mode, don't grab the cursor because
        // it is annoying when you hit a breakpoint and
        // can't move the mouse.
        window()->incInputCaptureCount();
    #endif
}


void UserInput::releaseMouse() {
    #ifndef G3D_DEBUG
        // In debug mode, don't grab the cursor because
        // it is annoying when you hit a breakpoint and
        // cannot move the mouse.
        window()->decInputCaptureCount();
    #endif

    // Restore the old mouse position
    setMouseXY(guiMouse);

    window()->decMouseHideCount();
}


void UserInput::setPureDeltaMouse(bool m) {
    if (_pureDeltaMouse != m) {
        _pureDeltaMouse = m;

        if (_pureDeltaMouse) {
            grabMouse();
        } else {
            releaseMouse();
        }
    }
}


}
