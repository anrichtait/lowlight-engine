/**
  \file G3D-app.lib/source/ControlPointEditor.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/ControlPointEditor.h"
#include "G3D-app/GuiButton.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/Draw.h"
#include "G3D-app/Surface.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/ThirdPersonManipulator.h"


static const float CONTROL_POINT_RADIUS = 0.2f;

namespace G3D {

void ControlPointEditor::ControlPointSurface::render
    (RenderDevice*                        rd, 
     const LightingEnvironment&           environment,
     RenderPassType                       passType) const {
    m_manipulator->m_mapper.update(rd);
    m_manipulator->renderControlPoints(rd, environment);
}


void ControlPointEditor::renderControlPoints
    (RenderDevice*                        rd, 
    const LightingEnvironment&            environment) const {

    for (int i = 0; i < numControlPoints(); ++i) {
        Draw::axes(controlPoint(i), rd);
    }
}


ControlPointEditor::ControlPointEditor(const GuiText& caption, GuiPane* dockPane, const shared_ptr<GuiTheme>& theme) : 
    GuiWindow(caption,
              theme,
              Rect2D::xywh(0,0,100,40), 
              GuiTheme::TOOL_WINDOW_STYLE,
              GuiWindow::HIDE_ON_CLOSE),
    m_selectedControlPointIndex(-1),
    m_lastNodeManipulatorControlPointIndex(-1),
    m_isDocked(dockPane != nullptr) {

    m_cachedPhysicsFrameString = CFrame(m_cachedPhysicsFrameValue).toAny().unparse();

    m_surface.reset(new ControlPointSurface(this));
    m_nodeManipulator = ThirdPersonManipulator::create();
    m_nodeManipulator->setEnabled(false);

    GuiPane* p = dockPane;

    if (p == nullptr) {
        // Place into the window
        p = pane();
    } else {
        // No need to show the window
        setVisible(false);
    }
    
    cpPane = p->addPane("Control Point", GuiTheme::ORNATE_PANE_STYLE);
    cpPane->moveBy(0, -15);

    m_addRemoveControlPointPane = cpPane->addPane("", GuiTheme::NO_PANE_STYLE);
    m_addRemoveControlPointPane->beginRow(); {
        GuiButton* b = m_addRemoveControlPointPane->addButton("Add new", this, &ControlPointEditor::addControlPoint);
        b->moveBy(-2, -7);
        m_removeSelectedButton = m_addRemoveControlPointPane->addButton("Remove", this, &ControlPointEditor::removeSelectedControlPoint);
    } m_addRemoveControlPointPane->endRow();

    m_selectedControlPointSlider = cpPane->addNumberBox("Index", &m_selectedControlPointIndex, "", GuiTheme::LINEAR_SLIDER, -1, 1);

    cpPane->addTextBox("", Pointer<String>(this, &ControlPointEditor::selectedNodePFrameAsString, &ControlPointEditor::setSelectedNodePFrameFromString));
    cpPane->pack();
    pack();

    setEnabled(false);
}


float ControlPointEditor::positionalEventZ(const Point2& pixel) const {
    int index = -1;
    return intersectRayThroughPixel(pixel, index);
}


float ControlPointEditor::intersectRayThroughPixel(const Point2& pixel, int& index) const {
    index = -1;
    const Ray& ray = m_mapper.eventPixelToCameraSpaceRay(pixel);

    float closest = finf();
    for (int i = 0; i < numControlPoints(); ++i) {
        const CFrame& c = controlPoint(i);

        const float t = ray.intersectionTime(Sphere(c.translation, CONTROL_POINT_RADIUS), true);
        if (t < closest) {
            index = i;
            closest = t;
        }
    }

    return closest / ray.direction().z;
}


bool ControlPointEditor::onEvent(const GEvent& event) {
    if (GuiWindow::onEvent(event)) {
        return true;
    }

    if ((event.type == GEventType::MOUSE_BUTTON_DOWN) && (event.button.button == 0) && ! event.button.controlKeyIsDown) {
        int index = -1;
        if (m_mapper.ready() && (intersectRayThroughPixel(event.mousePosition(), index) > -finf()) && (index >= 0)) {
            setSelectedControlPointIndex(index);
            return true;
        }
    }

    return false;
}


bool ControlPointEditor::hitsControlPoint(const Ray& r) {
    for (int i = 0; i < numControlPoints(); ++i) {
        const CFrame& c = controlPoint(i);
        if (r.intersectionTime(Sphere(c.translation, CONTROL_POINT_RADIUS), true) != finf()) {
            setSelectedControlPointIndex(i);
            return true;
        }
    }

    return false;
}


void ControlPointEditor::setEnabled(bool e) {
    GuiWindow::setEnabled(e);

    // If enabled, also make visible (so that the window can be seen)
    if (e && ! m_isDocked) {
        setVisible(true);
    }
}


String ControlPointEditor::selectedNodePFrameAsString() const {
    if ((m_selectedControlPointIndex >= 0) && (m_selectedControlPointIndex < numControlPoints())) {

        const PhysicsFrame& pframe = controlPoint(m_selectedControlPointIndex);

        // Cache the string so that we don't have to reparse it for every rendering
        if (m_cachedPhysicsFrameValue != pframe) {
            m_cachedPhysicsFrameValue = pframe;
            m_cachedPhysicsFrameString = CFrame(m_cachedPhysicsFrameValue).toAny().unparse();
        }

        return m_cachedPhysicsFrameString;

    } else {

        return "Point3(0, 0, 0)";

    }
}


void ControlPointEditor::setSelectedNodePFrameFromString(const String& s) {
    if ((m_selectedControlPointIndex >= 0) && (m_selectedControlPointIndex < numControlPoints())) {
        try {
            const PFrame& pframe = Any::parse(s);
            setControlPoint(m_selectedControlPointIndex, pframe);

            // Update the manipulator, so that it doesn't just override the value that we changed
            m_nodeManipulator->setFrame(pframe);
        } catch (...) {
            // Ignore parse errors
        }
    }
}


void ControlPointEditor::addControlPoint() {
    debugAssert(allowAddingAndRemovingControlPoints());
    if (numControlPoints() == 0) {
        addControlPointAfter(-1);
        resizeControlPointDropDown(numControlPoints());
        setSelectedControlPointIndex(0);
    } else if (numControlPoints() == 1) {
        addControlPointAfter(0);

        resizeControlPointDropDown(numControlPoints());
        // Select the new point
        setSelectedControlPointIndex(m_selectedControlPointIndex + 1);
    } else {
        addControlPointAfter(m_selectedControlPointIndex);
        // Select the new point
        resizeControlPointDropDown(numControlPoints());
        setSelectedControlPointIndex(m_selectedControlPointIndex + 1);        
    }
}


void ControlPointEditor::removeSelectedControlPoint() {
    if (numControlPoints() <= 1) {
        // Can't delete!
        return;
    }

    removeControlPoint(m_selectedControlPointIndex);
    setSelectedControlPointIndex(m_selectedControlPointIndex - 1);
    resizeControlPointDropDown(numControlPoints());
}


void ControlPointEditor::onPose(Array<shared_ptr<Surface> >& posedArray, Array<shared_ptr<Surface2D> >& posed2DArray) {
    if (enabled()) {
        posedArray.append(m_surface);
    }

    GuiWindow::onPose(posedArray, posed2DArray);
}


void ControlPointEditor::setManager(WidgetManager* m) {
    if ((m == nullptr) && (manager() != nullptr)) {
        // Remove controls from old manager
        manager()->remove(m_nodeManipulator);
    }

    GuiWindow::setManager(m);
    
    if (m != nullptr) {
        m->add(m_nodeManipulator);
    }
}


void ControlPointEditor::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    if (m_isDocked) {
        setVisible(false);
    }
    GuiWindow::onSimulation(rdt, sdt, idt);

    m_nodeManipulator->setEnabled(enabled());
    m_nodeManipulator->setTranslationEnabled(allowTranslation());
    m_nodeManipulator->setRotationEnabled(allowRotation());
    resizeControlPointDropDown(numControlPoints());

    m_addRemoveControlPointPane->setVisible(allowAddingAndRemovingControlPoints());
    
    if (enabled()) {
        if ((m_selectedControlPointIndex >= 0) && (m_selectedControlPointIndex < numControlPoints())) {
            if (m_lastNodeManipulatorControlPointIndex == m_selectedControlPointIndex) {
                // Move the control point to the manipulator
                setControlPoint(m_selectedControlPointIndex, m_nodeManipulator->frame());
            } else {
                // Move the manipulator to the control point
                m_nodeManipulator->setFrame(controlPoint(m_selectedControlPointIndex));
                m_lastNodeManipulatorControlPointIndex = m_selectedControlPointIndex;
            }
            m_removeSelectedButton->setEnabled(true);
            m_nodeManipulator->setEnabled(true);
        } else {
            m_removeSelectedButton->setEnabled(false);
            m_nodeManipulator->setEnabled(false);
        }
    }
}


void ControlPointEditor::setSelectedControlPointIndex(int i) {
    if ((i >= 0) && (i < numControlPoints())) {
        m_selectedControlPointIndex = i;
        // Move the manipulator to the new control point
        m_nodeManipulator->setFrame(controlPoint(m_selectedControlPointIndex));
        m_nodeManipulator->setEnabled(true);
    } else {
        m_nodeManipulator->setEnabled(false);
    }
}


void ControlPointEditor::resizeControlPointDropDown(int i) {
    m_selectedControlPointSlider->setRange(-1, i - 1);
}


void ControlPointEditor::renderManipulator(RenderDevice* rd) {
    if (m_nodeManipulator) {
        m_nodeManipulator->render(rd);
    }
}

}
