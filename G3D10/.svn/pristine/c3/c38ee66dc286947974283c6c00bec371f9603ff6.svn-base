/**
  \file G3D-app.lib/include/G3D-app/MarkerEntity.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_MarkerEntity_h

#include "G3D-base/platform.h"
#include "G3D-app/Entity.h"
#include "G3D-base/Box.h"
#include "G3D-base/Sphere.h"
#include "G3D-base/Color3.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-app/Scene.h"

namespace G3D {

class Scene;
class AnyTableReader;
class RenderDevice;
class Ray;

/** \brief A normally invisible Entity used for example, as a trigger,
    invisible collision, or spawn point.

    Although any Entity could be used in this way, MarkerEntity is
    especially supported for visualization and selection by the
    SceneEditorWindow.

    Syntax for use in a Scene.Any file:

    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    MarkerEntity {
         <all Entity properties>
         osBoxArray = [
            AABox(...),
            Box(...),
            ...
         ];
         color = Color4(...);
    }
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    \sa Light, Camera, VisibleEntity
*/
class MarkerEntity : public Entity {
protected:

    Array<Box>                  m_osBoxArray;
    Color3                      m_color;

    MarkerEntity();

    void init(AnyTableReader&    propertyTable);
    void init(const Array<Box>& boxArray, const Color3& color);

public:

    /** A translucent version of this color is used to visualize bounds. */
    const Color3& color() const {
        return m_color;
    }

    const Array<Box> osBoxArray() const {
        return m_osBoxArray;
    }

    static shared_ptr<MarkerEntity> create(const String& name);

    static shared_ptr<Entity> create 
        (const String&                          name,
         Scene*                                 scene,
         AnyTableReader&                        propertyTable,
         const ModelTable&                      modelTable = ModelTable(),
         const Scene::LoadOptions&              options    = Scene::LoadOptions());

    static shared_ptr<MarkerEntity> create
       (const String&                           name,
        Scene*                                  scene,
        const Array<Box>&                       osBoxArray  = Array<Box>(Box(Point3(-0.25f, -0.25f, -0.25f), Point3(0.25f, 0.25f, 0.25f))),
        const Color3&                           color       = Color3::white(),
        const CFrame&                           frame       = CFrame(),
        const shared_ptr<Track>&                track       = shared_ptr<Entity::Track>(),
        bool                                    canChange   = true,
        bool                                    shouldBeSaved = true);

    virtual Any toAny(const bool forceAll = false) const override;

    virtual void visualize(RenderDevice* rd, bool isSelected, const class SceneVisualizationSettings& s, const shared_ptr<GFont>& font, const shared_ptr<Camera>& camera) override;

    /** Updates the bounds */
    virtual void onSimulation(SimTime absoluteTime, SimTime deltaTime) override;

    /** Note that Scene::intersect will not invoke this method unless the intersectMarkers argument to that method is true.*/
    virtual bool intersect(const Ray& R, float& maxDistance, Model::HitInfo& info = Model::HitInfo::ignore) const override;
};

}
