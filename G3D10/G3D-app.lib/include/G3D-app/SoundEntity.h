/**
  \file G3D-app.lib/include/G3D-app/SoundEntity.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_app_SoundEntity_h

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
class Sound;
class AudioChannel;

/** \brief An invisilbe Entity that plays a positional sound and 
    removes itself from the Scene when the sound ends.

    Typically attached to another Entity via an Entity::EntityTrack.

    \sa MarkerEntity
*/
class SoundEntity : public Entity {
protected:

    /** Used when serializing */
    shared_ptr<Sound>                   m_sound;
    shared_ptr<AudioChannel>            m_audioChannel;

    /** Playing is triggered only on the first onSimulation, so that the position and
        velocity are correct */
    bool                                m_hadFirstSimulation = false;

    SoundEntity();

    void init(const shared_ptr<Sound>& sound, float initialVolume = 1.0f);
    void init(AnyTableReader&  propertyTable);
    
public:

    const shared_ptr<AudioChannel>& audioChannel() const {
        return m_audioChannel;
    }

    /** \sa audioChannel() */
    const shared_ptr<Sound>& sound() const {
        return m_sound;
    }

    /** Defaults to shouldBeSaved = false, canChange = true.

        \param name If empty, a unique name is automatically generated.

        \sa Entity::playSound */
    static shared_ptr<SoundEntity> create(const shared_ptr<Sound>& sound, float initialVolume = 1.0f, const String& name = "");

    /** For data-driven creation */
    static shared_ptr<Entity> create 
        (const String&                          name,
         Scene*                                 scene,
         AnyTableReader&                        propertyTable,
         const ModelTable&                      modelTable = ModelTable(),
         const Scene::LoadOptions&              options    = Scene::LoadOptions());

    virtual Any toAny(const bool forceAll = false) const override;

    virtual void visualize(RenderDevice* rd, bool isSelected, const class SceneVisualizationSettings& s, const shared_ptr<GFont>& font, const shared_ptr<Camera>& camera) override;

    /** Updates the AudioChannel::set3DAttributes */
    virtual void onSimulation(SimTime absoluteTime, SimTime deltaTime) override;

    /** Note that Scene::intersect will not invoke this method unless the intersectMarkers argument to that method is true.*/
    virtual bool intersect(const Ray& R, float& maxDistance, Model::HitInfo& info = Model::HitInfo::ignore) const override;
};

}
