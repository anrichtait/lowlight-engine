/**
  \file G3D-app.lib/source/SoundEntity.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-gfx/AudioDevice.h"
#include "G3D-app/SoundEntity.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/Camera.h"
#include "G3D-app/GFont.h"

#ifndef G3D_NO_FMOD

namespace G3D {

SoundEntity::SoundEntity() {}

void SoundEntity::init(AnyTableReader& propertyTable) {
    Any a;
    propertyTable.get("sound", a);
    const shared_ptr<Sound>& sound = Sound::create(a);
    float volume = 1.0f;
    propertyTable.getIfPresent("volume", volume);

    init(sound, volume);
}


void SoundEntity::init(const shared_ptr<Sound>& sound, float initialVolume) {
    if (sound) {
        m_sound = sound;
        m_audioChannel = sound->play(m_frame.translation, Vector3::zero(), initialVolume, -1.0f, true);
    }
}


shared_ptr<SoundEntity> SoundEntity::create(const shared_ptr<Sound>& sound, float initialVolume, const String& name) {
    static int count = 0;
    Any a(Any::TABLE);
    AnyTableReader reader(a);
    const shared_ptr<SoundEntity>& s = dynamic_pointer_cast<SoundEntity>(create(name.empty() ? format("%s%d", sound->name().c_str(), count++) : name, nullptr, reader, ModelTable()));
    s->audioChannel()->setVolume(initialVolume);
    s->setShouldBeSaved(false);
    s->m_canChange = true;

    return s;
}


shared_ptr<Entity> SoundEntity::create 
   (const String&                          name,
    Scene*                                 scene,
    AnyTableReader&                        propertyTable,
    const ModelTable&                      modelTable,
    const Scene::LoadOptions&              options) {

    const shared_ptr<SoundEntity>& m = createShared<SoundEntity>();
    m->Entity::init(name, scene, propertyTable);
    m->SoundEntity::init(propertyTable);
    propertyTable.verifyDone();
    return m;
}


Any SoundEntity::toAny(const bool forceAll) const {
    Any a = Entity::toAny(forceAll);
    a.setName("SoundEntity");    
    a["sound"] = m_sound->toAny();

    return a;
}


void SoundEntity::visualize(RenderDevice* rd, bool isSelected, const class SceneVisualizationSettings& s, const shared_ptr<GFont>& font, const shared_ptr<Camera>& camera) {
    if (s.showMarkers || isSelected) {
        // Only visualize the marker if showMarkers is set, to avoid confusion with bounding boxes
        // of geometry attached to markers.

        Entity::visualize(rd, isSelected, s, font, camera);
        
        rd->pushState(); {
            rd->setObjectToWorldMatrix(m_frame);
            rd->setProjectionAndCameraMatrix(camera->projection(), camera->frame());
            font->draw3DBillboard(rd, m_name, m_lastAABoxBounds.center(), m_lastAABoxBounds.extent().length() * 0.1f, Color3::white());
        } rd->popState();
    }
}


void SoundEntity::onSimulation(SimTime absoluteTime, SimTime deltaTime) {
    Entity::onSimulation(absoluteTime, deltaTime);

    if (m_hadFirstSimulation && m_audioChannel->done()) {
        m_scene->remove(dynamic_pointer_cast<Entity>(shared_from_this()));
    } else {
        const Vector3& velocity = 
            (isFinite(deltaTime) && (deltaTime > 0.0f)) ?
                (m_frame.translation - m_previousFrame.translation) / deltaTime :
                Vector3::zero();

        m_audioChannel->set3DAttributes(m_frame.translation, velocity);
    }

    if (! m_hadFirstSimulation) {
        // Unpause on first simulation
        m_hadFirstSimulation = true;
        m_audioChannel->setPaused(false);
    }
}


bool SoundEntity::intersect(const Ray& R, float& maxDistance, Model::HitInfo& info) const {
    return intersectBounds(R, maxDistance, info);
}


} // namespace
#endif
