/**
  \file G3D-app.lib/include/G3D-app/Model.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_Model_h

#include "G3D-base/platform.h"
#include "G3D-base/Table.h"
#include "G3D-base/lazy_ptr.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-app/Surface.h"
#include "G3D-app/Material.h"

namespace G3D {

class Entity;

/** Common base class for models */
class Model : public ReferenceCountedObject {
private:

    static bool s_useOptimizedIntersect;

public:

    /** \sa Scene::intersect, Entity::intersect, ArticulatedModel::intersect, Tri::Intersector.
         All fields are const so as to require the use of the set method to set the value of the field
         while still keeping all fields public. */
    class HitInfo {
    public:

        /** In world space.  Point3::nan() if no object was hit. */
        const Point3                   point;

        /** In world space */
        const Vector3                  normal;

        /** May be null */
        const shared_ptr<Entity>       entity;

        const shared_ptr<Model>        model;

        /** May be null */
        const shared_ptr<Material>     material;

        /** If the model contains multiple meshes (e.g.,
            ArticulatedModel), this is an identifier for the
            underlying mesh or other surface in which primitiveIndex
            should be referenced.*/
        const String                   meshName;

        const String                   partName;

        /** For debugging */
        const int                      meshID;

        /** If the model has multiple primitives, this is the index of the one hit */
        const int                      primitiveIndex;

        /** Barycentric coords within the primitive hit if it is a triangle.*/
        const float                    u;

        /** Barycentric coords within the primitive hit if it is a triangle.*/
        const float                    v;
        
        static HitInfo                 ignore;

        HitInfo();

        void clear(); 

        void set
        (const shared_ptr<Model>&    model,
         const shared_ptr<Entity>&   entity       = nullptr,
         const shared_ptr<Material>& material     = nullptr,
         const Vector3&              normal       = Vector3::nan(),
         const Point3&               point        = Point3::nan(),
         const String&               partName     = "",
         const String&               meshName     = "",
         int                         meshID       = 0,
         int                         primIndex    = 0,
         float                       u            = 0.0f,
         float                       v            = 0.0f);

    };

    /** Information for converting a single frame of a Model to a Surface. */
    class Pose : public ReferenceCountedObject {
    public:
        Pose() {}

        /** Returns true if other is null, has a different type than this, or would cause a Model of the appropriate type 
            to change its bounding boxes. */
        virtual bool differentBounds(const shared_ptr<Pose>& other) const = 0;
        virtual ~Pose() {}
        virtual shared_ptr<Pose> clone() const = 0;
    };
    
    /** Name of the instance (usually based on the filename it is loaded from) */
    virtual const String& name() const = 0;

    /** Name of the G3D::Model subclass */
    virtual const String& className() const = 0;

    /**  This will be replaced soon with a version that takes a shared_ptr<Pose>.

        \param pose Must have the subclass of Model::Pose appropriate to the Model subclass.
      */
    virtual void pose
    (Array<shared_ptr<Surface> >&   surfaceArray, 
     const CFrame&                  rootFrame, 
     const CFrame&                  prevFrame, 
     const shared_ptr<Entity>&      entity,
     const Model::Pose*             pose,
     const Model::Pose*             prevPose,
     const Surface::ExpressiveLightScatteringProperties& e) = 0;

    /**
        Determines if the ray intersects the heightfield and
        fills the \a info with the proper information.

        \param maxDistance Max distance to trace to on input, hit distance written on output if hit
    */
    virtual bool intersect
    (const Ray&                      ray, 
     const CoordinateFrame&          cframe, 
     float&                          maxDistance, 
     Model::HitInfo&                 info           = Model::HitInfo::ignore,
     const Entity*                   entity         = nullptr,
     const Model::Pose*              pose           = nullptr) const {
        return false;
    }

    /** \sa setUseOptimizedIntersect */
    static bool useOptimizedIntersect() {
        return s_useOptimizedIntersect;
    }

    /** If true, complex models should use TriTree to accelerate intersect() calls where possible.
        This can make the first intersect() call very slow for the tree build and can make loading 
        slow. It may not affect performance of skinned or articulated models that animate.

        This value should be set before the models are loaded. If it is changed after a model is loaded,
        the Model is not required to respond to it.

        Default: false */
    static void setUseOptimizedIntersect(bool b) {
        s_useOptimizedIntersect = b;
    }
};


typedef Table< String, lazy_ptr<Model> >  ModelTable;

} // namespace G3D
