/**
  \file G3D-app.lib/include/G3D-app/XRWidget.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#define G3D_app_XRWidget_h

#include "G3D-base/platform.h"
#include "G3D-gfx/XR.h"
#include "G3D-app/Widget.h"
#include "G3D-app/MarkerEntity.h"

namespace G3D {

class Scene;

/**
    \brief A G3D::Widget that manages world-space G3D::MarkerEntity%s based on polling a low-level XR instance.

    Manages MarkerEntities for:

    - "XR Tracked Volume"
    - "XR Left Hand"
    - "XR Left Controller"
    - "XR Right Hand"
    - "XR Right Controller"
    - "XR Head"
    - "XR Body"
    - Other tracked objects as "XR Tracked Object ##"

    The body entity is currently synthesized from the head and hand positions. It is approximately the 
    center of mass of the torso.

    Attach the camera to the head entity and geometry to the hand entities to work with GApp. To
    put the player in a moving reference frame, you can attach the "XR Tracked Volume" entity to
    a vehicle.

    Everything in XR is relative to the tracked volume, so to teleport, move the "XR Tracked Volume",
    not the player.
*/
class XRWidget : public Widget {
public:

    class TrackedEntity : public MarkerEntity {
    protected:
        shared_ptr<XR::Object> m_object;
        shared_ptr<Entity>     m_trackedVolume;

        TrackedEntity(const String& name, Scene* scene, const AABox& bounds, const Color3& color, const shared_ptr<XR::Object>& object, const shared_ptr<Entity>& volume);

    public:

        static shared_ptr<TrackedEntity> create
            (const String&                  name,
            Scene*                         scene,
            const AABox&                   bounds,
            const Color3&                  color,
            const shared_ptr<XR::Object>&  object,
            const shared_ptr<Entity>&      volume);

        virtual void onSimulation(SimTime absoluteTime, SimTime deltaTime) override;

        const shared_ptr<XR::Object>& object() const {
            return m_object;
        }
    };


    /** 
      Blinks used for gaze-directed user interfaces. 

      Due to their use of higher level data structures, these are not G3D::GEvents.
      A future redesign of the event system may allow high level events like this.

      Poll using XRWidget::pollBlinkEvent

      \sa XRWidget, GazeTracker, GazeTracker::Gaze::inBlink
      \beta */
    class VoluntaryBlinkEvent {
    public:
        G3D_DECLARE_ENUM_CLASS(Type, LONG_BLINK, DOUBLE_BLINK);

        /** LONG_BLINK for a single blink of unusual duration. 
            DOUBLE_BLINK for exactly two consecutive blinks, each of slightly longer duration than expected 
            for involuntary blinks */
        Type                type;

        /** World-space gaze ray at the time that the blink started. */
        Ray                 gazeRay[2];

        /** World-space gaze point at the time that the blink started.
        
          \see XRWidget::robustBinocularGazePoint */
        Point3              point;
    };

protected:
    friend class VRApp;

    shared_ptr<XR>                  m_xrSystem;
    class GApp*                     m_app;
    shared_ptr<MarkerEntity>        m_trackedVolume;
    shared_ptr<MarkerEntity>        m_body;

    /** What the facing direction was according to the hands the last time that the hands were moving slowly. */
    Vector3                         m_latchedFacingFromHands = Vector3::nan();

    /** The last time either hand moved quickly. */
    RealTime                        m_lastFastHandMovementTime = 0.0f;

    Table<String, shared_ptr<TrackedEntity>> m_entityTable;
    shared_ptr<Entity>              m_controllerEntity[2];

    /** Camera not visible to the normal scene graph that is updated every frame based on m_eyeFrame.
        The GApp::activeCamera() is bound to one of these during GApp::onGraphics.

        \sa m_eyeFrame */
    shared_ptr<Camera>              m_vrEyeCamera[2];

    /** In world space. Updated by processGazeTracker */
    Ray                             m_gazeRay[2];

    /** This is NaN when it needs to be updated. Mutable because 
        it us updated on the first call to the const accessor. */
    mutable Point3                  m_gazePoint;

    /** If currently tracking a LONG_BLINK, this is the time at which it began. */
    RealTime                        m_blinkStartTime = nan();

    /** The gaze rays before the user blinked when m_blinkStartTime != nan */
    Ray                             m_preBlinkGazeRay[2];

    static const int MAX_BLINK_QUEUE_SIZE = 4;
    Queue<VoluntaryBlinkEvent>      m_blinkEventQueue;

    XRWidget(const shared_ptr<XR>& xr, GApp* app);

    void simulateBodyFrame(RealTime rdt);

    void processGazeTracker();

    Point3 robustBinocularGazePoint(const Ray gazeRay[2]) const;

public:

    const shared_ptr<Camera>& cameraForView(int viewIndex) const {
        debugAssert(viewIndex >= 0 && viewIndex < 2);
        return m_vrEyeCamera[viewIndex];
    }

    /** @deprecated use cameraForView */
    const shared_ptr<Camera>& eyeCamera(int index) const {
        return cameraForView(index);
    }
           
    static shared_ptr<XRWidget> create(const shared_ptr<XR>& xr, GApp* app);

    /** Returns the height above ground of the top of the first player's head. */
    float standingHeadHeight() const;

    // We sample positional trackers in onSimulation because GApp executes
    // Widget::onSimulation before Scene::onSimulation, and thus the entities
    // managed will be updated correctly for the current frame.

    /** Samples from the XR and then creates or updates the MarkerEntity%s in the Scene as
        needed. */
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;

    /** Updates the HMD cameras based on the current
        GApp::activeCamera()'s parameters, as well as the gaze data. */
    virtual void onBeforeGraphics() override;

    /** World-space gaze ray for the given eye, reported directly from the GazeTracker.
        The result is latched immediately before onGraphics runs.
        
        \sa GApp::gazeTracker, XRWidget::projectedMonocularGazePoint, XRWidget::robustBinocularGazePoint, GazeTracker */
    virtual Ray gazeRay(int eyeIndex) const;

    /** 2D screen-space view point for one eye.
    
       Useful for visualizing the raw results from a GazeTracker.
       Projection of the ws gaze ray into the view of the given index
       by assuming that they have the same center of projection. This is the
       case (or at least, a good approximation!) for a HMD with two views
       when eyeIndex matches the primary display for that eye. This 
       will not necessarily work for wide field of view HMDs that use multiple
       projections. This is not the projection of robustGazePoint.
       The result is latched immediately before onGraphics runs. 

       \sa GApp::gazeTracker, XRWidget::gazeRay, XRWidget::robustBinocularGazePoint, GazeTracker
       */
    virtual Point2 projectedMonocularGazePoint(int eyeIndex, int viewIndex, const Rect2D& viewport) const;

    /** World space 3D gaze point inferred from the scene and the gaze rays. This is
        is probably what the user is actually looking at in 3D.
        The result is semantically latched immediately before onGraphics runs,
        but is only computed on demand because of the ray casts involved.
        This may be slow if not using VRApp or Model::useOptimizedIntersect() == true.

        \sa GApp::gazeTracker, XRWidget::projectedMonocularGazePoint, XRWidget::gazeRay, GazeTracker
        */
    virtual Point3 robustBinocularGazePoint() const;

    /** \beta Gets the next blink event from the queue, returning false if there was not one. 
       The queue saturates at a small number of entries, since many gaze tracking programs
       might ignore these events. */
    virtual bool pollBlinkEvent(VoluntaryBlinkEvent& event); 
};

} // G3D

