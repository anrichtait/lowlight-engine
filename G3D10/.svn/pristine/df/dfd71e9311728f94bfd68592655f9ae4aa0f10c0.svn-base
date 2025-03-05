/**
  \file G3D-app.lib/source/XRWidget.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-app/XRWidget.h"
#include "G3D-app/GApp.h"
#include "G3D-app/Scene.h"
#include "G3D-app/VRApp.h"
#include "G3D-base/FileSystem.h"

namespace G3D {

static const float bodyWidth          = 0.35f;
static const float bodyThickness      = 0.19f;

/** Horizontal offset from the center of the "head" (HMD) to the center of the body. */
static const float neckOffset         = 0.15f;

/** From the center of the head to the center of the "body", vertically */
static const float headToBodyDistance = 0.35f;

/** Length of the body */
static const float bodyLength = 0.45f;


static const float longBlinkTime = 0.15f; // ms

bool XRWidget::pollBlinkEvent(VoluntaryBlinkEvent& event) {
    if (m_blinkEventQueue.size() > 0) {
        event = m_blinkEventQueue.popFront();
        return true;
    } else {
        return false;
    }
}

XRWidget::XRWidget(const shared_ptr<XR>& xr, GApp* app) : m_xrSystem(xr), m_app(app) {
    for (int eye = 0; eye < 2; ++eye) {
        static const String NAME[2] = { "G3D::VRApp::m_vrEyeCamera[0]", "G3D::VRApp::m_vrEyeCamera[1]" };
        Any a(Any::TABLE);
        a["overridePixelOffset"] = false;
        AnyTableReader propTable(a);
        m_vrEyeCamera[eye] = dynamic_pointer_cast<Camera>(Camera::create(NAME[eye], m_app->scene().get(), propTable));
        m_vrEyeCamera[eye]->setShouldBeSaved(false);
    }
}


shared_ptr<XRWidget> XRWidget::create(const shared_ptr<XR>& xr, GApp* app) {
    return createShared<XRWidget>(xr, app);
}


float XRWidget::standingHeadHeight() const {
    const shared_ptr<XRWidget::TrackedEntity>& head = m_app->scene()->typedEntity<XRWidget::TrackedEntity>("XR Head");
    if (notNull(head)) {
        const shared_ptr<XR::HMD>& hmd = dynamic_pointer_cast<XR::HMD>(head->object());
        if (notNull(hmd)) {
            return hmd->standingHeadHeight();
        }
    }

    // Default fall through
    return 1.8f;
}


void XRWidget::simulateBodyFrame(RealTime rdt) {
    const shared_ptr<Entity>& head = m_app->scene()->entity("XR Head");
    const shared_ptr<Entity>& left = m_app->scene()->entity("XR Left Controller");
    const shared_ptr<Entity>& right = m_app->scene()->entity("XR Right Controller");

    if (notNull(head)) {
        const CFrame   oldBodyFrame    = m_body->frame();
        const CFrame   headFrame       = head->frame();
        const Vector3& oldLookNoPitch  = (oldBodyFrame.lookVector() * Vector3(1, 0, 1)).direction();
        const Vector3& headLookNoPitch = (headFrame.lookVector() * Vector3(1, 0, 1)).direction();
        bool isBodyTurning = false;

        // What the facing direction would be if we only knew the hands
        Vector3 facingFromHands;
        if (left && right) {
            // 1 / time
            const float idt = 1.0f / float(rdt);

            // Get the left hand's speed in m/s
            const Vector3& leftMove = left->frame().translation - left->previousFrame().translation;
            const float leftSpeed = leftMove.length() * idt; // m/s

            // Get the right hand's speed in m/s
            const Vector3& rightMove = right->frame().translation - right->previousFrame().translation;
            const float rightSpeed = rightMove.length() * idt; // m/s

            // Only update when the hands are still...otherwise, the player might just be reaching temporarily.
            const RealTime timeToWaitAfterFastMovement = 0.35f; // s
            static const float slowSpeed     = 0.975f; // m/s
            static const float verySlowSpeed = 0.0975f; // m/s

            // Calculate turn of head in radians
            const Vector3& oldHeadLook = head->previousFrame().lookVector();
            float headDiffAngle = atan2(headLookNoPitch.x, headLookNoPitch.z) - atan2(oldHeadLook.x, oldHeadLook.z);

            // Calculate angular displacement of left hand in radians
            const Vector3& oldLeft = left->previousFrame().translation - head->previousFrame().translation;
            const Vector3& newLeft = left->frame().translation - head->frame().translation;
            const float leftDiffAngle = atan2(newLeft.x, newLeft.z) - atan2(oldLeft.x, oldLeft.z);

            // Calculate angular displacement of right hand in radians
            const Vector3& oldRight = right->previousFrame().translation - head->previousFrame().translation;
            const Vector3& newRight = right->frame().translation - head->frame().translation;
            const float rightDiffAngle = atan2(newRight.x, newRight.z) - atan2(oldRight.x, oldRight.z);

            // Tolerance for angular displacement
            const float angTol = 2.0f * units::degrees();

            // Body is turning if angular displacement is similar
            isBodyTurning = (abs(headDiffAngle - leftDiffAngle) < angTol) && (abs(headDiffAngle - rightDiffAngle) < angTol);

            // Assume body isn't turning if one of the hands is not moving
            isBodyTurning = isBodyTurning && (leftSpeed > verySlowSpeed) && (rightSpeed > verySlowSpeed);

            const RealTime now = System::time();
            // If hands are moving fast, and that movement is not part of the whole body turning, use previous hand positions
            // This means the body frame will not move when doing hand gestures, but will when turning the whoe body
            if ((max(leftSpeed, rightSpeed) >= slowSpeed) && !isBodyTurning) {
                m_lastFastHandMovementTime = now;
            }

            // If we are not doing hand gestures, update body with current hand positions
            if ((now > m_lastFastHandMovementTime + timeToWaitAfterFastMovement) || m_latchedFacingFromHands.isNaN()) {

                // Since we average hand positions, if one or both is behind body, we get the wrong results, so we reflect them across the local x-axis
                //  For the purposes of detecting whether hands are behind body, checking whether they are behind the head is a good 
                //  enough approximation

                // Use headframe independant of pitch for better approximation of what is behind the body (body doesnt pitch with head)
                //Vector3 unitLook = headFrame.lookVector().directionOrZero();
                //Vector3 unitUp = headFrame.upVector().directionOrZero();
                CFrame usedHeadFrame = CFrame(Matrix3::fromColumns(headFrame.rightVector(), Vector3::unitY(), -Vector3::unitY().cross(headFrame.rightVector().directionOrZero())), headFrame.translation);

                // Get right and left in head space
                Vector3 rightInHeadSpace = usedHeadFrame.vectorToObjectSpace(right->frame().translation - usedHeadFrame.translation);
                Vector3 leftInHeadSpace = usedHeadFrame.vectorToObjectSpace(left->frame().translation - usedHeadFrame.translation);

                Vector3 usedLeft = ((left->frame().translation - usedHeadFrame.translation) * Vector3(1, 0, 1)).direction();
                Vector3 usedRight = ((right->frame().translation - usedHeadFrame.translation) * Vector3(1, 0, 1)).direction();
                
                // When right hand is  behind the 'body', reflect right across x axis
                if (rightInHeadSpace.z > 0.0f){
                    usedRight = rightInHeadSpace * Vector3(1, 0, -1).direction();
                    usedRight = usedHeadFrame.vectorToWorldSpace(usedRight);
                }

                // When left hand is behind the 'body', reflect left across x axis
                if (leftInHeadSpace.z > 0.0f){
                    usedLeft = leftInHeadSpace * Vector3(1, 0, -1).direction();
                    usedLeft = usedHeadFrame.vectorToWorldSpace(usedLeft);
                }

                m_latchedFacingFromHands = facingFromHands = (usedLeft + usedRight).direction();

                if (facingFromHands.isNaN()) {
                    m_latchedFacingFromHands = facingFromHands = (usedHeadFrame.lookVector() * Vector3(1, 0, 1)).directionOrZero();
                   
                }
                debugAssert(!facingFromHands.isNaN());
            } else {
                facingFromHands = m_latchedFacingFromHands;
                debugAssert(!facingFromHands.isNaN());
            }

        } else {
            facingFromHands = headLookNoPitch;
            debugAssert(!facingFromHands.isNaN());
        }

        // Always lerp. Works fine for rapid teleportation and masks small errors in tracking data or body simulation
        // Body frame rigidly moving always feels off, lerp is much less distressing
        Vector3 newLook = oldLookNoPitch.lerp((headLookNoPitch + facingFromHands * 4.0f).direction(), 0.04f).directionOrZero();
        debugAssert(!newLook.isNaN());
        if (newLook.squaredLength() < 0.5f) {
            // worst case, happens during initialization sometimes
            newLook = headLookNoPitch;
        }

        // Lerp if the new and old position are reasonably close to avoid too much jerking around. 
        // If the player has moved dramatically, then something is wrong (probably a teleport), so snap directly instead.
        // Decrease the lerp threshold when looking up or down, which makes it easier to spin.
        //static const float threshold = isBodyTurning ? cos(180 * units::degrees()) : cos(360 * units::degrees());
        //if (oldLookNoPitch.dot(newLook) > threshold * sqrt(abs(headFrame.lookVector().y))) {
        //    debugAssert(!newLook.isNaN());
        //    newLook = oldLookNoPitch.lerp((headLookNoPitch + facingFromHands * 4.0f).direction(), 0.04f).directionOrZero();
        //}

        CFrame bodyFrame;
        //If body would clip with the ground, assume crawling
        if (headFrame.translation.y < bodyLength) {
            const Vector3& up = Vector3::unitY();
            //Create the body frame such that the neck points along calculated 'newLook' to simulate a crawling body
            bodyFrame = CFrame(Matrix3::fromColumns(newLook.cross(up), newLook, -up));
            bodyFrame.rotation.orthonormalize();

            debugAssert(! isNaN(bodyFrame.rotation.determinant()));
        
            bodyFrame.translation = headFrame.translation - newLook*headToBodyDistance - headFrame.upVector() * neckOffset;
        } else {
            const Vector3& up = Vector3::unitY();
            bodyFrame = CFrame(Matrix3::fromColumns(newLook.cross(up), up, -newLook));
            bodyFrame.rotation.orthonormalize();

            debugAssert(!isNaN(bodyFrame.rotation.determinant()));

            bodyFrame.translation = headFrame.translation - Vector3(0.0f, headToBodyDistance, 0.0f) - headLookNoPitch * neckOffset;
        }

        debugAssert(bodyFrame.rotation.isOrthonormal());
        m_body->setFrame(bodyFrame);
    }
}


void XRWidget::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    static const String controllerEntityName[2] = {"XRWidget Right Controller", "XRWidget Left Controller"};
    m_xrSystem->updateTrackingData();

    const shared_ptr<Scene>& scene = m_app->scene();

    if (isNull(m_trackedVolume)) {
        // Create the tracked volume first, because everything else depends on it
        m_trackedVolume = MarkerEntity::create("XR Tracked Volume");
        m_trackedVolume->setShouldBeSaved(false);
        scene->insert(m_trackedVolume);

        // Create the body frame, awaiting the head and hands
        m_body = MarkerEntity::create("XR Body", m_app->scene().get(), Array<Box>(AABox(Point3(-bodyWidth / 2.0f, -0.2f, -bodyThickness / 2.0f), Point3(bodyWidth / 2.0f, 0.3f, bodyThickness / 2.0f))), Color3::cyan());
        m_trackedVolume->setShouldBeSaved(false);
        scene->insert(m_body);
    } else if (isNull(scene->entity(m_trackedVolume->name()))) {
        // The scene must have been changed or reloaded and the XR entities are no longer present.
        // Immediately add them back in.
        scene->insert(m_trackedVolume);
        if (isNull(scene->entity(m_body->name()))) {
            scene->insert(m_body);
        }

        for (Table<String, shared_ptr<TrackedEntity>>::Iterator it = m_entityTable.begin(); it.hasMore(); ++it) {
            scene->insert(it.value());
            scene->setOrder(m_trackedVolume->name(), it.key());
        }
    }

    debugAssert(m_trackedVolume->frame().rotation.isOrthonormal());

    // Process changes to the m_objectTable
    for (shared_ptr<XR::Event> event = m_xrSystem->nextEvent(); notNull(event); event = m_xrSystem->nextEvent()) {
        const shared_ptr<XR::Object>& object = event->object;
        switch (event->type) {
        case XR::Event::OBJECT_CREATED:
        {
            // This is invoked for the HMD and controllers

            // HTC Vive wand bounding box
            AABox bounds(Vector3(-0.055f, -0.04f, -0.05f), Vector3(0.055f, 0.04f, 0.15f));
            const Color3& color = Color3::cyan();
            const shared_ptr<TrackedEntity>& entity = TrackedEntity::create(object->name(), m_app->scene().get(), bounds, color, event->object, m_trackedVolume);
            const shared_ptr<Scene>& scene = m_app->scene();

            scene->insert(entity);
            scene->setOrder(m_trackedVolume->name(), entity->name());
            m_entityTable.set(event->object->name(), entity);

            if (object->isController() && scene->vrSettings().avatar.addControllerEntity) {
                const shared_ptr<XR::Controller>& controller = dynamic_pointer_cast<XR::Controller>(object);
                const int index = controller->isRight() ? 1 : 0;

                // Use the filename as the model name to make it unique
                const String& modelFilename = FilePath::canonicalize(controller->modelFilename());
                if (! scene->modelTable().containsKey(modelFilename)) {
                    scene->createModel(Any(modelFilename), modelFilename);
                }

                const String& parentName = controller->isRight() ? "XR Right Controller" : "XR Left Controller";
                m_controllerEntity[index] = scene->createEntity(controllerEntityName[index], Any::parse(format(STR(VisibleEntity {
                    model = "%s"; 
                    track = entity("%s"); 
                    shouldBeSaved = false;
                    }), modelFilename.c_str(), parentName.c_str())));
            } // if add controller          

        }
        break;

        case XR::Event::OBJECT_DESTROYED:
            m_app->scene()->remove(m_entityTable[event->object->name()]);
            m_entityTable.remove(event->object->name());
            break;

        }
    }

    // Re-add controllers to scene as needed
    if (scene->vrSettings().avatar.addControllerEntity) {
        for (int i = 0; i < 2; ++i) {
            if (notNull(m_controllerEntity[i]) && isNull(scene->entity(controllerEntityName[i]))) {
                scene->insert(m_controllerEntity[i]);
            }
        } // i
    }

    simulateBodyFrame(rdt);
}


void XRWidget::onBeforeGraphics() {
    // Update the virtual cameras
    if (isNull(m_app) || isNull(m_app->activeCamera()) || isNull(m_app->scene())) {
        return;
    }

    const shared_ptr<Camera>& activeCamera = m_app->activeCamera();
    const shared_ptr<XRWidget::TrackedEntity>& head = m_app->scene()->typedEntity<XRWidget::TrackedEntity>("XR Head");
    alwaysAssertM(notNull(head), "Could not find entity XR Head");
    const shared_ptr<XR::HMD>& hmd = dynamic_pointer_cast<XR::HMD>(head->object());

    // Move the eye cameras for each HMD
    CFrame eyeToHead[2];
    Projection projection[2];
    const float nearZ = activeCamera->nearPlaneZ(), farZ = activeCamera->farPlaneZ();
    hmd->getViewCameraMatrices(nearZ, farZ, eyeToHead, projection);

    for (int eye = 0; eye < 2; ++eye) {
        // Save old matrices
        const CFrame oldFrame = m_vrEyeCamera[eye]->frame();

        m_vrEyeCamera[eye]->copyParametersFrom(activeCamera);
        m_vrEyeCamera[eye]->setOverridePixelOffset(false);

        if (m_app->settings().vr.overrideMotionBlur && m_vrEyeCamera[eye]->motionBlurSettings().enabled()) {
            m_vrEyeCamera[eye]->motionBlurSettings() = m_app->settings().vr.motionBlurSettings;
        }

        if (m_app->settings().vr.overrideDepthOfField && m_vrEyeCamera[eye]->depthOfFieldSettings().enabled()) {
            m_vrEyeCamera[eye]->depthOfFieldSettings() = m_app->settings().vr.depthOfFieldSettings;
        }

        m_vrEyeCamera[eye]->setFrame(head->frame() * eyeToHead[eye]);
        m_vrEyeCamera[eye]->setPreviousFrame(oldFrame);

        m_vrEyeCamera[eye]->setProjection(projection[eye]);
        m_vrEyeCamera[eye]->setPreviousProjection(projection[eye]);
    }

    // Debugging: print the eye translations
    /*
    for (int eye = 0; eye < numEyes(); ++eye) {
        screenPrintf("%s\n", m_vrEyeCamera[eye]->frame().translation.toString().c_str());
    }
    */

    processGazeTracker();
}

/////////////////////////////////////////////////////////////////////////

XRWidget::TrackedEntity::TrackedEntity
   (const String&                  name,
    Scene*                         scene,
    const AABox&                   bounds,
    const Color3&                  color,
    const shared_ptr<XR::Object>&  object,
    const shared_ptr<Entity>&      volume) :
    m_object(object),
    m_trackedVolume(volume) {

    alwaysAssertM(this != volume.get(), "The tracked volume cannot be a TrackedEntity.");

    Entity::init(name, scene, CFrame(), nullptr, true, false);
    MarkerEntity::init(Array<Box>(bounds), color);
    setShouldBeSaved(false);
}


shared_ptr<XRWidget::TrackedEntity> XRWidget::TrackedEntity::create
   (const String&                        name,
    Scene*                               scene,
    const AABox&                         bounds,
    const Color3&                        color,
    const shared_ptr<XR::Object>&        object,
    const shared_ptr<Entity>&            volume) {
    alwaysAssertM(notNull(object), "Null XR::Object");
    alwaysAssertM(notNull(volume), "Null tracked volume Entity");
    return createShared<TrackedEntity>(name, scene, bounds, color, object, volume);
}


void XRWidget::TrackedEntity::onSimulation(SimTime absoluteTime, SimTime deltaTime) {
    MarkerEntity::onSimulation(absoluteTime, deltaTime);
    m_frame = m_trackedVolume->frame() * m_object->frame();
}

////////////////////////////////////////////////////////////////////////////////////

void XRWidget::processGazeTracker() {
    const CFrame& headFrame = m_app->headFrame();

    // Detect blink event beginning and save the previous gaze rays before
    // entering the blink.

    const bool inBlink = (m_app->gazeForEye(0).inBlink || m_app->gazeForEye(1).inBlink);
    bool blinkJustStarted = false;

    if (inBlink && isNaN(m_blinkStartTime) && (m_blinkEventQueue.size() < MAX_BLINK_QUEUE_SIZE)) {
        // Start the blink
        m_blinkStartTime = System::time();
        blinkJustStarted = true;

    } else if (! inBlink && ! isNaN(m_blinkStartTime)) {
        // End the blink
        const float duration = float(System::time() - m_blinkStartTime);

        if (duration > longBlinkTime) {
            // Generate a long blink event. 

            // Cast the ray now. It would have been better to cast the ray against the
            // scene at the start time, but at the start time we didn't know that this would
            // be a long blink, and didn't want to spend the CPU time if it wasn't.

            VoluntaryBlinkEvent event;
            event.type = VoluntaryBlinkEvent::Type::LONG_BLINK;
            for (int eye = 0; eye < 2; ++eye) {
                event.gazeRay[eye] = m_preBlinkGazeRay[eye];
            }
            event.point = robustBinocularGazePoint(m_preBlinkGazeRay);
            m_blinkEventQueue.pushBack(event);
        }

        m_blinkStartTime = nan();
    }
    

    for (int eye = 0; eye < 2; ++eye) {
        const Ray& hsRay = m_app->gazeForEye(eye).headSpaceRay;
        const Ray& wsRay = headFrame.toWorldSpace(hsRay);
        m_gazeRay[eye] = wsRay;

        if (! inBlink) {
            // Don't update once the blink has started.
            // During the blink the data are synthetic and when the blink ends
            // the result may be unreliable for a few frames, so the event 
            // needs the rays from before the blink.
            m_preBlinkGazeRay[eye] = wsRay;
        }
    }

    m_gazePoint = Point3::nan();
}


Ray XRWidget::gazeRay(int eyeIndex) const {
    return m_gazeRay[eyeIndex];
}


Point2 XRWidget::projectedMonocularGazePoint(int eyeIndex, int viewIndex, const Rect2D& viewport) const {
    const shared_ptr<Camera>& camera = cameraForView(viewIndex);

    // Assume that the optical axis of the eye returned from the gaze tracker 
    // is parallel to the projection camera used for that view
    const Ray& csRay = camera->frame().toObjectSpace(m_gazeRay[eyeIndex]);
    const Point2& ssPoint = camera->projection().project(csRay.direction().xyz(), viewport).xy();

    return ssPoint.xy();
}


Point3 XRWidget::robustBinocularGazePoint(const Ray gazeRay[2]) const {
    // Update the point
    Point3 gazePoint;

    float distance[2] = {finf(), finf()};

    // Prevent the ray cast from hitting geometry attached to the head itself
    Array<shared_ptr<Entity>> excludeArray;
    for (const String& e : {"XR Head", "(Active Camera Marker)"}) {
        excludeArray.append(m_app->scene()->entity(e));
    }

    // Ray cast
    for (int eye = 0; eye < 2; ++eye) {
        m_app->scene()->intersect(gazeRay[eye], distance[eye], false, excludeArray);
    }

    if (isFinite(distance[0])) {
        if (isFinite(distance[1])) {
            // Both eyes hit finite points. Average them
            Point3 P;
            for (int eye = 0; eye < 2; ++eye) {
                P += gazeRay[eye].origin() + gazeRay[eye].direction() * distance[eye];
            }
            gazePoint = P / 2.0f;
        } else {
            // Eye 0 is finite
            gazePoint = gazeRay[0].origin() + gazeRay[0].direction() * distance[0];
        }
    } else if (isFinite(distance[1])) {
        // Eye 1 is finite
        gazePoint = gazeRay[1].origin() + gazeRay[1].direction() * distance[1];
    } else {
        // Neither eye is finite. Assume an average point far away (optical infinity)
        const float focusDistance = 20.0f;
        for (int eye = 0; eye < 2; ++eye) {
            gazePoint += gazeRay[eye].origin() + gazeRay[eye].direction() * focusDistance;
        }
        gazePoint /= 2.0f;
    }

    return gazePoint;
}


Point3 XRWidget::robustBinocularGazePoint() const {
    if (m_gazePoint.isNaN()) {
        m_gazePoint = robustBinocularGazePoint(m_gazeRay);
    }

    return m_gazePoint;
}

} // namespace G3D
