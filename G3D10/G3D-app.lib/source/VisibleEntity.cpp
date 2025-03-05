/**
  \file G3D-app.lib/source/VisibleEntity.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/VisibleEntity.h"
#include "G3D-base/Box.h"
#include "G3D-base/AABox.h"
#include "G3D-base/Sphere.h"
#include "G3D-base/Ray.h"
#include "G3D-base/LineSegment.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/Pointer.h"
#include "G3D-app/HeightfieldModel.h"
#include "G3D-app/Draw.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/Scene.h"
#include "G3D-app/GuiPane.h"
#include "G3D-app/GApp.h"
#include "G3D-app/ParticleSystemModel.h"
#include "G3D-app/ParticleSystem.h"

namespace G3D {

shared_ptr<Entity> VisibleEntity::create 
   (const String&                           name,
    Scene*                                  scene,
    AnyTableReader&                         propertyTable,
    const ModelTable&                       modelTable,
    const Scene::LoadOptions&               options) {

        
    bool canChange = false;
    propertyTable.getIfPresent("canChange", canChange);
    // Pretend that we haven't peeked at this value
    propertyTable.setReadStatus("canChange", false);

    if ((canChange && ! options.stripDynamicVisibleEntitys) || 
        (! canChange && ! options.stripStaticVisibleEntitys)) {

        shared_ptr<VisibleEntity> visibleEntity(new VisibleEntity());
        visibleEntity->Entity::init(name, scene, propertyTable);
        visibleEntity->VisibleEntity::init(propertyTable, modelTable);
        propertyTable.verifyDone();

        return visibleEntity;
    } else {
        return nullptr;
    }
}


shared_ptr<VisibleEntity> VisibleEntity::create
   (const String&                           name,
    Scene*                                  scene,
    const shared_ptr<Model>&                model,
    const CFrame&                           frame,
    const shared_ptr<Entity::Track>&        track,
    bool                                    canChange,
    bool                                    shouldBeSaved,
    bool                                    visible,
    const Surface::ExpressiveLightScatteringProperties& expressiveLightScatteringProperties,
    const ArticulatedModel::PoseSpline&     artPoseSpline,
    const shared_ptr<Model::Pose>&          pose) {

    const shared_ptr<VisibleEntity>& visibleEntity = createShared<VisibleEntity>();

    visibleEntity->Entity::init(name, scene, frame, track, canChange, shouldBeSaved);
    visibleEntity->VisibleEntity::init(model, visible, expressiveLightScatteringProperties, artPoseSpline, MD3Model::PoseSequence(), pose);
     
    return visibleEntity;
}


VisibleEntity::VisibleEntity() : Entity(), m_visible(true) {
    m_canCauseCollisions = true;
}


void VisibleEntity::init
   (const shared_ptr<Model>&                model,
    bool                                    visible,
    const Surface::ExpressiveLightScatteringProperties& expressiveLightScatteringProperties,
    const ArticulatedModel::PoseSpline&     artPoseSpline,
    const MD3Model::PoseSequence&           md3PoseSequence,
    const shared_ptr<Model::Pose>&          pose) {

    m_artPoseSpline = artPoseSpline;
    m_md3PoseSequence = md3PoseSequence;
    m_visible = visible;
    
    setModel(model);
    m_pose = pose;
    m_previousPose = pose;

    m_expressiveLightScatteringProperties = expressiveLightScatteringProperties;
}


void VisibleEntity::init
   (AnyTableReader&    propertyTable,
    const ModelTable&  modelTable) {

    bool visible = true;
    propertyTable.getIfPresent("visible", visible);
    
    ArticulatedModel::PoseSpline artPoseSpline;
    propertyTable.getIfPresent("poseSpline", artPoseSpline);

    MD3Model::PoseSequence md3PoseSequence;
    propertyTable.getIfPresent("md3Pose", md3PoseSequence);

    Surface::ExpressiveLightScatteringProperties expressiveLightScatteringProperties;
    propertyTable.getIfPresent("expressiveLightScatteringProperties", expressiveLightScatteringProperties);
    
    // Override entity defaults
    bool c = true;
    propertyTable.setReadStatus("canCauseCollisions", false);
    propertyTable.getIfPresent("canCauseCollisions", c);
    setCanCauseCollisions(c);

    if (propertyTable.getIfPresent("castsShadows", expressiveLightScatteringProperties.castsShadows)) {
    //    debugPrintf("Warning: castsShadows field is deprecated.  Use expressiveLightScatteringProperties");
    }    

    const lazy_ptr<Model>* model = nullptr;
    Any modelNameAny;
    if (propertyTable.getIfPresent("model", modelNameAny)) {
        const String& modelName     = modelNameAny.string();
    
        modelNameAny.verify(modelTable.containsKey(modelName), 
                        "Can't instantiate undefined model named " + modelName + ".");
        model = modelTable.getPointer(modelName);
    }

    const shared_ptr<Model>& mmodel = notNull(model) ? model->resolve() : nullptr;

    shared_ptr<Model::Pose> pose;
    Any ap;
    if (propertyTable.getIfPresent("articulatedModelPose", ap)) {
        pose = ArticulatedModel::Pose::create(ap);
    } else if (dynamic_pointer_cast<ArticulatedModel>(mmodel)) {
        // No pose present
        pose = ArticulatedModel::Pose::create();
    } else if (dynamic_pointer_cast<MD2Model>(mmodel)) {
        // No pose present
        pose = MD2Model::Pose::create();
    } else if (dynamic_pointer_cast<MD3Model>(mmodel)) {
        // No pose present
        pose = MD3Model::Pose::create();
    }
    
    Any ignore;
    if (propertyTable.getIfPresent("materialTable", ignore)) {
        ignore.verify(false, "'materialTable' is deprecated. Specify materials on the articulatedModelPose field of VisibleEntity.");
    }

    init(mmodel, visible, expressiveLightScatteringProperties, artPoseSpline, md3PoseSequence, pose);
}


void VisibleEntity::setModel(const shared_ptr<Model>& model) {
    m_model = model;

    alwaysAssertM(isNull(dynamic_pointer_cast<ParticleSystemModel>(m_model)) ||
        notNull(dynamic_cast<ParticleSystem*>(this)),
        "ParticleSystemModel must be used with ParticleSystem. It cannot be used with VisibleEntity.");
    
    m_lastChangeTime = System::time();
}


void VisibleEntity::setPose(const shared_ptr<Model::Pose>& pose) {
    if (isNull(pose)) {
        // Removing pose
        m_pose = nullptr;
        m_previousPose = nullptr;
    } else if (pose != m_pose) {
        // New non-null pose object that is different than before
        if (notNull(m_pose)) { 
            m_previousPose = m_pose;
        } else {
            // Previously had no pose, so use a clone of the new one
            m_previousPose = pose->clone();
        }
        m_pose = pose;
    } else if (isNull(m_previousPose)) {
        // Setting back to the same non-null value and we have no previous,
        // so synthesize one
        m_previousPose = m_pose->clone();
    }

    debugAssert(notNull(m_pose) == notNull(m_previousPose));
}


void VisibleEntity::simulatePose(SimTime absoluteTime, SimTime deltaTime) {
    // Legacy support for animating specific models efficiently. This
    // will eventually be moved into Model::Pose itself.
    const shared_ptr<ArticulatedModel::Pose>& artPose         = dynamic_pointer_cast<ArticulatedModel::Pose>(m_pose);
    const shared_ptr<ArticulatedModel::Pose>& artPreviousPose = dynamic_pointer_cast<ArticulatedModel::Pose>(m_previousPose);

    const shared_ptr<MD2Model::Pose>& md2Pose         = dynamic_pointer_cast<MD2Model::Pose>(m_pose);
    const shared_ptr<MD3Model::Pose>& md3Pose         = dynamic_pointer_cast<MD3Model::Pose>(m_pose);

    if (notNull(artPose)) {
        debugAssertM(notNull(artPreviousPose), "Pose set on an ArticulatedModel without a previous pose.");

        const shared_ptr<ArticulatedModel>& artModel = dynamic_pointer_cast<ArticulatedModel>(m_model);
        if (isNaN(deltaTime) || (deltaTime > 0)) {
            artPreviousPose->frameTable = artPose->frameTable;
            artPreviousPose->uniformTable = artPose->uniformTable;
            if (notNull(artPose->uniformTable)) {
                artPose->uniformTable = UniformTable::createShared<UniformTable>(*artPreviousPose->uniformTable);
            }
        }

        if (artModel->usesSkeletalAnimation()) {
            Array<String> animationNames;
            artModel->getAnimationNames(animationNames);
            ArticulatedModel::Animation animation;
            artModel->getAnimation(animationNames[0], animation);
            animation.getCurrentPose(absoluteTime, *artPose);
        } else {
            m_artPoseSpline.get(float(absoluteTime), *artPose);
        }

        // Intentionally only compare cframeTables; materialTables don't change (usually)
        // and are more often non-empty, which could trigger a lot of computation here.
        if (artPreviousPose->frameTable != artPose->frameTable) {
            m_lastChangeTime = System::time();
        }
    } else if (notNull(md2Pose)) {
        MD2Model::Pose::Action a;
        md2Pose->onSimulation(deltaTime, a);
        if (isNaN(deltaTime) || (deltaTime > 0)) {
            m_lastChangeTime = System::time();
        }
    } else if (notNull(md3Pose)) {
        m_md3PoseSequence.getPose(float(absoluteTime), *md3Pose);
        dynamic_pointer_cast<MD3Model>(m_model)->simulatePose(*md3Pose, deltaTime);
        if (isNaN(deltaTime) || (deltaTime > 0)) {
            m_lastChangeTime = System::time();
        }
    }
}


void VisibleEntity::onSimulation(SimTime absoluteTime, SimTime deltaTime) {
    Entity::onSimulation(absoluteTime, deltaTime);
    simulatePose(absoluteTime, deltaTime);
}


void VisibleEntity::poseModel(Array<shared_ptr<Surface> >& surfaceArray) const {
    if (isNull(m_model)) { return; }
    const shared_ptr<Entity>& me = dynamic_pointer_cast<Entity>(const_cast<VisibleEntity*>(this)->shared_from_this());
    m_model->pose(surfaceArray, m_frame, m_previousFrame, me, m_pose.get(), m_previousPose.get(), m_expressiveLightScatteringProperties);
}


void VisibleEntity::onPose(Array<shared_ptr<Surface> >& surfaceArray) {

    // We have to pose in order to compute bounds that are used for selection in the editor
    // and collisions in simulation, so pose anyway if not visible,
    // but then roll back.
    debugAssert(isFinite(m_frame.translation.x));
    debugAssert(! isNaN(m_frame.rotation[0][0]));
    const int oldLen = surfaceArray.size();

    poseModel(surfaceArray);
    const bool boundsChangedSincePreviousFrame = (m_frame != m_previousFrame) || (notNull(m_pose) && m_pose->differentBounds(m_previousPose));

    // Compute bounds for objects that moved
    if (m_lastAABoxBounds.isEmpty() || boundsChangedSincePreviousFrame || (m_lastChangeTime > m_lastBoundsTime)) {

        m_lastSphereBounds = Sphere(m_frame.translation, 0);
        
        const CFrame& myFrameInverse = frame().inverse();

        m_lastObjectSpaceAABoxBounds = AABox::empty();
        m_lastBoxBoundArray.fastClear();

        // Look at all surfaces produced
        for (int i = oldLen; i < surfaceArray.size(); ++i) {
            AABox b;
            Sphere s;
            const shared_ptr<Surface>& surf = surfaceArray[i];

            // body to world transformation for the surface
            CoordinateFrame cframe;
            surf->getCoordinateFrame(cframe, false);
            debugAssertM(cframe.translation.x == cframe.translation.x, "NaN translation");


            surf->getObjectSpaceBoundingSphere(s);
            s = cframe.toWorldSpace(s);
            m_lastSphereBounds.radius = max(m_lastSphereBounds.radius,
                                            (s.center - m_lastSphereBounds.center).length() + s.radius);

            // Take the entity's frame out of consideration, so that we get tight AA bounds 
            // in the Entity's frame
            CFrame osFrame = myFrameInverse * cframe;

            surf->getObjectSpaceBoundingBox(b);

            if (b.isEmpty()) {
                b = AABox(Point3::zero());
            }
            m_lastBoxBoundArray.append(cframe.toWorldSpace(b));
            const Box& temp = osFrame.toWorldSpace(b);
            m_lastObjectSpaceAABoxBounds.merge(temp);
        }

        // Box can't represent an empty box, so we make empty boxes into real boxes with zero volume here
        if (m_lastObjectSpaceAABoxBounds.isEmpty()) {
            m_lastObjectSpaceAABoxBounds = AABox(Point3::zero());
            m_lastAABoxBounds = AABox(frame().translation);
        }

        m_lastBoxBounds = frame().toWorldSpace(m_lastObjectSpaceAABoxBounds);
        m_lastBoxBounds.getBounds(m_lastAABoxBounds);
        m_lastBoundsTime = System::time();
    }

    if (! m_visible) {
        // Discard my surfaces if I'm invisible; they were only needed for bounds
        surfaceArray.resize(oldLen, false);
    }
}

#if 0
void VisibleEntity::debugDrawVisualization(RenderDevice* rd, VisualizationMode mode) {
    alwaysAssertM(mode == SKELETON, "Bounds visualization not implemented");
    Array<Point3> skeletonLines;
    switch (m_modelType) {
    case ARTICULATED_MODEL:
        m_artModel->getSkeletonLines(m_artPose, m_frame, skeletonLines);
        break;
    default:
        break;
    }
    rd->pushState(); {
        rd->setObjectToWorldMatrix(CFrame());
        rd->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);
        for (int i = 0; i < skeletonLines.size(); i += 2) {
            Draw::lineSegment(LineSegment::fromTwoPoints(skeletonLines[i], skeletonLines[i+1]), rd, Color3::red());
        }
    } rd->popState();
}
#endif


bool VisibleEntity::intersect(const Ray& R, float& maxDistance, Model::HitInfo& info) const {
    if (m_model && m_visible) {
        return m_model->intersect(R, m_frame, maxDistance, info, this, m_pose.get());
    } else {
        return false;
    }
}


bool VisibleEntity::intersectBounds(const Ray& R, float& maxDistance, Model::HitInfo& info) const {
    if (m_model && m_visible) {
        return Entity::intersectBounds(R, maxDistance, info);
    } else {
        return false;
    }    
}


Any VisibleEntity::toAny(const bool forceAll) const {
    Any a = Entity::toAny(forceAll);
    a.setName("VisibleEntity");

    AnyTableReader oldValues(a);
    bool visible;
    if (forceAll || (oldValues.getIfPresent("visible", visible) && visible != m_visible)) {
        a.set("visible", m_visible);
    }

    // Model and pose must already have been set, so no need to change anything
    return a;
}


void VisibleEntity::onModelDropDownAction() {
    const String& choice = m_modelDropDownList->selectedValue().text();
    if (choice == "<none>") {
        setModel(shared_ptr<Model>());
    } else {
        // Parse the name
        const size_t i = choice.rfind('(');
        const String& modelName = choice.substr(0, i - 1);

        // Find the model with that name
        const lazy_ptr<Model>* model = m_scene->modelTable().getPointer(modelName);
        if (notNull(model)) {
            setModel(model->resolve());
        } else {
            setModel(shared_ptr<Model>());
        } // not null
    }
}


void VisibleEntity::makeGUI(class GuiPane* pane, class GApp* app) {
    Entity::makeGUI(pane, app);

    Array<String> modelNames("<none>");
    int selected = 0;
    for (ModelTable::Iterator it = m_scene->modelTable().begin(); it.hasMore(); ++it) {      
        const lazy_ptr<Model>& m = it->value;

        if (m.resolved()) {
            modelNames.append(it->key + " (" + it->value.resolve()->className() + ")");
        } else {
            // The model type is unknown because it hasn't been loaded yet
            modelNames.append(it->key);
        }

        if (it->value == m_model) {
            selected = modelNames.size() - 1;
        }
    }
    m_modelDropDownList = pane->addDropDownList("Model", modelNames, nullptr, GuiControl::Callback(this, &VisibleEntity::onModelDropDownAction));
    m_modelDropDownList->setSelectedIndex(selected);
    pane->addCheckBox("Visible", &m_visible);
}


}
