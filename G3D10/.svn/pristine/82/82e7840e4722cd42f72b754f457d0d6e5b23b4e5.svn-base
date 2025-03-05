/**
  \file G3D-app.lib/include/G3D-app/GameController.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_GameController_h

#include "G3D-base/platform.h"
#include "G3D-base/Vector2.h"
#include "G3D-gfx/GKey.h"
#include "G3D-app/Widget.h"

namespace G3D {

class OSWindow;


/**
Platform-independent tracking of input from an Xbox360 controller.

The Xbox360 controller has become the de facto standard PC controller and
merits special support in G3D.  Unforuntately, the controller's axes and buttons are mapped
differently on Windows and OS X by the underlying drivers.  This class 
provides a uniform interface.

A reliable open source OS X driver for the Xbox360/XboxOne controller is available from
https://github.com/360Controller/360Controller/releases and a Wireless Gaming Receiver for Windows 
is available from http://tattiebogle.net/index.php/ProjectRoot/GameController/OsxDriver

There is no hardware difference between the "Xbox360 controller for Windows" by Microsoft and the 
"Xbox360 controller" that ships for the console itself.  However, third party controllers
may not work with the Microsoft driver for Windows. A driver that the G3D team has used 
successfully with these controllers is available (with source) at:
http://vba-m.com/forum/Thread-xbcd-0-2-7-release-info-updates-will-be-posted-here (follow the installation instructions carefully).

On Windows, the left and right trigger buttons are mapped to the same axis due to
a strange underlying API choice by Microsoft in their own driver and DirectInput 8.  
The newer XInput API supports the axes correctly, and force feedback.  Since G3D 9.0, 
G3D uses the 
GLFW library for access to the joystick. G3D
will provide independent access to the triggers when the GLFW project adds support
for XInput.

\sa UserInput, UserInput::virtualStick1, OSWindow::getJoystickState

*/
class GameController : public Widget {
protected:

    class Button {
    public:
        bool                    currentValue;

        /** Changed since the previous onAfterEvents */
        bool                    changed;

        Button() : currentValue(false), changed(false) {}
    };

    class Stick {
    public:
        Vector2                 currentValue;
        Vector2                 previousValue;
    };

    bool                        m_present;
    unsigned int                m_joystickNumber;

    enum {NUM_STICKS = 6, NUM_BUTTONS = GKey::CONTROLLER_GUIDE - GKey::CONTROLLER_A + 1};

    Stick                       m_stickArray[NUM_STICKS];

    /** State of the buttons, where <code>index = k - GKey::CONTROLLER_A</code>*/
    Button                      m_buttonArray[NUM_BUTTONS];

    /** Performs range checking */
    const Stick& stick(JoystickIndex) const;

    /** Performs range checking */
    Button& button(GKey k);

    const Button& button(GKey k) const {
        return const_cast<GameController*>(this)->button(k);
    }

    GameController(unsigned int n) : m_present(false), m_joystickNumber(n) {}

public:

    /** True if this controller is connected and appears to actually be an Xbox360 controller. */
    bool present() const {
        return m_present;
    }

    static shared_ptr<GameController> create(unsigned int joystickNumber) {
        return createShared<GameController>(joystickNumber);
    }

    virtual void setManager(WidgetManager* m) override;

    /** Latches the state of the controller. */
    virtual void onAfterEvents() override;
 
    /** Returns true if this controller button was pressed between the last two calls of onAfterEvents.  Supports
        GKey::CONTROLLER_A through GKey::CONTROLLER_GUIDE. */
    bool justPressed(GKey k) const {
        return button(k).currentValue && button(k).changed;
    }

    /** Returns true if this controller button was held down as of the last onAfterEvents call.  Supports
        GKey::CONTROLLER_A through GKey::CONTROLLER_GUIDE. */
    bool currentlyDown(GKey k) const {
        return button(k).currentValue;
    }

    /** Returns true if this controller button was released between the last two calls of onAfterEvents.  Supports
        GKey::CONTROLLER_A through GKey::CONTROLLER_GUIDE. */
    bool justReleased(GKey k) const {
        return ! button(k).currentValue && button(k).changed;
    }

    /** Position of an analog stick as of onAfterEvents. */
    Vector2 position(JoystickIndex s) const {
        return stick(s).currentValue;
    }

    /** Change in position of an analog stick between the previous two calls to onAfterEvents. */
    Vector2 delta(JoystickIndex s) const { 
        const Stick& st = stick(s);
        return st.currentValue - st.previousValue;
    }

    /** 
      Returns the counter-clockwise angle in radians that the stick has rotated through between the last
      two calls to onAfterEvents.  This is zero if the stick position had magnitude less than 0.20 during
      the frame.
      Useful for gesture-based input, such as the spray-painting swipes in Jet Grind Radio.
    */
    float angleDelta(JoystickIndex s) const;
};

} // namespace
