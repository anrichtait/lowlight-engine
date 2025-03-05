/**
  \file G3D-app.lib/include/G3D-app/VisibleEntity.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_VisibleEntity_h

#include "G3D-base/platform.h"
#include "G3D-app/Entity.h"
#include "G3D-app/ArticulatedModel.h"
#include "G3D-app/MD2Model.h"
#include "G3D-app/MD3Model.h"
#include "G3D-app/Scene.h"

namespace G3D {

class HeightfieldModel;
class Scene;

/** \brief Base class for Entity%s that use a built-in G3D::Model subclass. 

    \sa G3D::Light, G3D::Camera, G3D::MarkerEntity, G3D::Scene
*/
class VisibleEntity : public Entity {
public:
    enum VisualizationMode {
        SKELETON,
        BOUNDS,
        SKELETON_AND_BOUNDS
    };

protected:

    class GuiDropDownList*          m_modelDropDownList;

    /** GUI callback */
    void onModelDropDownAction();
    
    Surface::ExpressiveLightScatteringProperties m_expressiveLightScatteringProperties;

    shared_ptr<Model>               m_model;
    shared_ptr<Model::Pose>         m_previousPose;
    shared_ptr<Model::Pose>         m_pose;

    /** Pose over time. */
    ArticulatedModel::PoseSpline    m_artPoseSpline;

    MD3Model::PoseSequence          m_md3PoseSequence;

    /** Should this Entity currently be allowed to affect any part of the rendering pipeline (e.g., shadows, primary rays, indirect light)?  If false, 
    the Entity never returns any surfaces from onPose(). Does not necessarily mean that the underlying model is visible to primary rays.*/
    bool                            m_visible;

    VisibleEntity();

    /** \sa create */
    void init
        (AnyTableReader&                                              propertyTable,
         const ModelTable&                                            modelTable);

    /** \sa create */
    void init
       (const shared_ptr<Model>&                                      model,
        bool                                                          visible,
        const Surface::ExpressiveLightScatteringProperties&           expressiveLightScatteringProperties,
        const ArticulatedModel::PoseSpline&                           artPoseSpline,
        const MD3Model::PoseSequence&                                 md3PoseSequence = MD3Model::PoseSequence(),
        const shared_ptr<Model::Pose>&                                pose = nullptr);

    /** Animates the appropriate pose type for the model selected.
     Called from onSimulation.  Subclasses will frequently replace
     onSimulation but retain this helper method.*/
    virtual void simulatePose(SimTime absoluteTime, SimTime deltaTime);
    
    /** Called from VisibleEntity::onPose to extract surfaces from the model.
        If you subclass VisibleEntity to support a new model type, override 
        this method. onPose will then still compute bounds correctly for
        your model.

        \return True if the surfaces returned have different bounds than in the previous
        frame, e.g., due to animation.
    */
    virtual void poseModel(Array<shared_ptr<Surface> >& surfaceArray) const;

public:

   /** \brief Construct a VisibleEntity.

       \param name The name of this Entity, e.g., "Player 1"

       \param propertyTable The form is given below.  It is intended that
       subclasses replace the table name and add new fields.

       \code
       VisibleEntity {
           model        = <modelname>;
           poseSpline   = <ArticulatedModel::PoseSpline>;
           articulatedModelPose = <ArticulatedModel::Pose>;
           md3Pose      = <MD2Model::PoseSequence>;
           visible      = <bool>;
           expressiveLightScatteringProperties = <object>;
           ... // all Entity properties
       }
       \endcode

       In particular, to pass a custom Shader argument (see also G3D::Args and G3D::UniformTable) for a VisibleEntity,
       use:

       \code
       VisibleEntity {
          ...
          articulatedModelPose = ArticulatedModel::Pose {
             ...
             uniformTable = {
                 myShaderArg = <value>;
             }
          };
       }
       \endcode

       See also the properties from Entity::init.

       expressiveLightScatteringProperties are stored on the pose object for the specific model chosen so that multiple instances of the
       same model can exist with different shadow-casting properties.

       The poseSpline and castsShadows fields are optional.  The Entity base class
       reads these fields.  Other subclasses read their own fields.  Do not specify a poseSpline
       unless you want to override the defaults.  Specifying them reduces rendering performance slightly.

       \param modelTable Maps model names that are referenced in \a propertyTable
       to shared_ptr<ArticulatedModel>, shared_ptr<MD2Model> or MD3::ModelRef.
       
       \param scene May be nullptr

       The original caller (typically, a Scene class) should invoke
       AnyTableReader::verifyDone to ensure that all of the fields 
       specified were read by some subclass along the inheritance chain.       

       See samples/starter/source/Scene.cpp for an example of use.
     */
    static shared_ptr<Entity> create 
        (const String&                           name,
         Scene*                                  scene,
         AnyTableReader&                         propertyTable,
         const ModelTable&                       modelTable,
         const Scene::LoadOptions&               options);

    static shared_ptr<VisibleEntity> create
       (const String&                           name,
        Scene*                                  scene,
        const shared_ptr<Model>&                model,
        const CFrame&                           frame         = CFrame(),
        const shared_ptr<Track>&                track         = shared_ptr<Entity::Track>(),
        bool                                    canChange     = true,
        bool                                    shouldBeSaved = true,
        bool                                    visible       = true,
        const Surface::ExpressiveLightScatteringProperties& expressiveLightScatteringProperties = Surface::ExpressiveLightScatteringProperties(),
        const ArticulatedModel::PoseSpline&     artPoseSpline = ArticulatedModel::PoseSpline(),
        const shared_ptr<Model::Pose>&          pose          = nullptr);

    /** Converts the current VisibleEntity to an Any.  Subclasses should
        modify at least the name of the Table returned by the base class, which will be "Entity"
        if not changed. */
    virtual Any toAny(const bool forceAll = false) const override;

    /** Not all VisibleEntity subclasses will accept all models. If this model is not appropriate for this subclass,
        then the model() will not change. */
    virtual void setModel(const shared_ptr<Model>& model);
    
    /** 
     Invokes poseModel to compute the actual surfaces and then computes bounds on them when needed.
     \sa Entity::onPose
    */
    virtual void onPose(Array<shared_ptr<Surface> >& surfaceArray) override;

    virtual void onSimulation(SimTime absoluteTime, SimTime deltaTime) override;

    virtual bool intersect(const Ray& R, float& maxDistance, Model::HitInfo& info = Model::HitInfo::ignore) const override;
    virtual bool intersectBounds(const Ray& R, float& maxDistance, Model::HitInfo& info) const override;

    bool visible() const {
        return m_visible;
    }

    virtual void setVisible(bool b) {
        m_visible = b;
    }

    void setCastsShadows(bool b) {
        m_expressiveLightScatteringProperties.castsShadows = b;
    }

    bool castsShadows() const {
        return m_expressiveLightScatteringProperties.castsShadows;
    }

    const Surface::ExpressiveLightScatteringProperties& expressiveLightScatteringProperties() const {
        return m_expressiveLightScatteringProperties;
    }

    const shared_ptr<Model>& model() const {
        return m_model;
    }

    /** \deprecated */
    ArticulatedModel::Pose& articulatedModelPose() {
        return *dynamic_pointer_cast<ArticulatedModel::Pose>(m_pose);
    }

    virtual void setPose(const shared_ptr<Model::Pose>& pose);

    virtual const shared_ptr<Model::Pose>& pose() const {
        return m_pose;
    }

    virtual void makeGUI(class GuiPane* pane, class GApp* app) override;

};

}
