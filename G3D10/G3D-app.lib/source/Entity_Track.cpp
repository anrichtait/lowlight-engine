/**
  \file G3D-app.lib/source/Entity_Track.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/Any.h"
#include "G3D-app/Entity.h"
#include "G3D-app/Scene.h"

namespace G3D {

class CombineTrack : public Entity::Track {
protected:
    shared_ptr<Entity::Track>      m_rotation;
    shared_ptr<Entity::Track>      m_translation;
        
    CombineTrack(const shared_ptr<Entity::Track>& r, const shared_ptr<Entity::Track>& t) : 
        m_rotation(r), m_translation(t) {}
        
public:

    static shared_ptr<CombineTrack> create(const shared_ptr<Entity::Track>& r, const shared_ptr<Entity::Track>& t) {
        return createShared<CombineTrack>(r, t);
    }
        
    virtual CFrame computeFrame(SimTime time) const override {
        return CFrame
            (m_rotation->computeFrame(time).rotation, 
                m_translation->computeFrame(time).translation);
    }
        
};
    
    
class TransformTrack : public Entity::Track {
public:
    shared_ptr<Entity::Track>      a;
    shared_ptr<Entity::Track>      b;

protected:
    TransformTrack(const shared_ptr<Entity::Track>& a, const shared_ptr<Entity::Track>& b) : 
        a(a), b(b) {}

public:

    static shared_ptr<TransformTrack> create(const shared_ptr<Entity::Track>& a, const shared_ptr<Entity::Track>& b) {
        return createShared<TransformTrack>(a, b);
    }

    virtual CFrame computeFrame(SimTime time) const override {
        return a->computeFrame(time) * b->computeFrame(time); 
    }
};


class OrbitTrack : public Entity::Track {
public:
    float                           radius;
    float                           period;

protected:
    OrbitTrack(float r, float p) : radius(r), period(p) {}

public:

    static shared_ptr<OrbitTrack> create(float r, float p) {
        return createShared<OrbitTrack>(r, p);
    }

    virtual CFrame computeFrame(SimTime time) const override {
        const float angle = float(twoPi() * time) / period;
        return CFrame::fromXYZYPRRadians(sin(angle) * radius, 0, cos(angle) * radius, angle);
    }
};



Entity::EntityTrack::EntityTrack(Entity* entity, const String& n, Scene* scene, const CFrame& frame) : m_entityName(n), m_childFrame(frame), m_scene(scene) {
    alwaysAssertM(notNull(scene), "Scene may not be null");
    alwaysAssertM(notNull(entity), "Entity may not be null");
    m_scene->setOrder(n, entity->name());
}


/** The entity cannot be changed once the track is created, but the relative frame may be changed */ 
const String& Entity::EntityTrack::entityName() const {
    return m_entityName;
}


const CFrame& Entity::EntityTrack::childFrame() const {
    return m_childFrame;
}


void Entity::EntityTrack::setChildFrame(const CFrame& frame) {
    m_childFrame = frame;
}
        

shared_ptr<Entity::EntityTrack> Entity::EntityTrack::create(Entity* entity, const String& n, Scene* scene, const CFrame& childFrame) {
    return createShared<Entity::EntityTrack>(entity, n, scene, childFrame);
}


CFrame Entity::EntityTrack::computeFrame(SimTime time) const {
    const shared_ptr<Entity>& e = m_scene->entity(m_entityName);
    if (notNull(e)) {
        return e->frame() * m_childFrame;
    } else {
        // Maybe in initialization and the other entity does not yet exist
        return m_childFrame;
    }
}


class LookAtTrack : public Entity::Track {
protected:
    shared_ptr<Entity::Track>      m_base;
    shared_ptr<Entity::Track>      m_target;
    const Vector3                  m_up;

    LookAtTrack(const shared_ptr<Entity::Track>& base, const shared_ptr<Entity::Track>& target, const Vector3& up) : 
        m_base(base), m_target(target), m_up(up) {}

public:

    static shared_ptr<LookAtTrack> create(const shared_ptr<Entity::Track>& base, const shared_ptr<Entity::Track>& target, const Vector3& up) {
        return createShared<LookAtTrack>(base, target, up);
    }

    virtual CFrame computeFrame(SimTime time) const override {
        CFrame f = m_base->computeFrame(time);
        f.lookAt(m_target->computeFrame(time).translation, m_up);
        return f;
    }
};


class TimeShiftTrack : public Entity::Track {
protected:
    shared_ptr<Entity::Track>           m_track;
    SimTime                             m_dt;

    TimeShiftTrack(const shared_ptr<Entity::Track>& c, const SimTime dt) : 
        m_track(c), m_dt(dt) {}

public:
    
    static shared_ptr<TimeShiftTrack> create(const shared_ptr<Entity::Track>& c, const SimTime dt) {
        return createShared<TimeShiftTrack>(c, dt);
    }

    virtual CFrame computeFrame(SimTime time) const override {
        return m_track->computeFrame(time + m_dt);
    }

};

void Entity::Track::VariableTable::set(const String& id, const shared_ptr<Entity::Track>& val) {
    debugAssert(notNull(val));
    variable.set(id, val);
}


shared_ptr<Entity::Track> Entity::Track::VariableTable::operator[](const String& id) const {
    const shared_ptr<Entity::Track>* valPtr = variable.getPointer(id);

    if (isNull(valPtr)) {
        if (isNull(parent)) {
            // Variable not found.  We do not allow Entity names here because it would prevent static 
            // checking of ID's...the full list of Entitys is unknown when parsing their Tracks.
            return shared_ptr<Entity::Track>();
        } else {
            // Defer to parent
            return (*parent)[id];
        }
    } else {
        // Return the value found
        return *valPtr;
    }
}


shared_ptr<Entity::Track> Entity::Track::create(Entity* entity, Scene* scene, const Any& a) {
    VariableTable table;
    return create(entity, scene, a, table);
}


shared_ptr<Entity::Track> Entity::Track::create(Entity* entity, Scene* scene, const Any& a, const VariableTable& variableTable) {
    if (a.type() == Any::STRING) {
        // This must be an id
        const shared_ptr<Entity::Track>& c = variableTable[a.string()];
        if (isNull(c)) {
            a.verify(false, "");
        }
        return c;
    }

    if ((beginsWith(a.name(), "PhysicsFrameSpline")) ||
        (beginsWith(a.name(), "PFrameSpline")) ||
        (beginsWith(a.name(), "Point3")) || 
        (beginsWith(a.name(), "Vector3")) || 
        (beginsWith(a.name(), "Matrix3")) || 
        (beginsWith(a.name(), "Matrix4")) || 
        (beginsWith(a.name(), "CFrame")) || 
        (beginsWith(a.name(), "PFrame")) || 
        (beginsWith(a.name(), "UprightSpline")) || 
        (beginsWith(a.name(), "CoordinateFrame")) || 
        (beginsWith(a.name(), "PhysicsFrame"))) {

        return Entity::SplineTrack::create(a);

    } else if (a.name() == "entity") {

        // Name of an Entity
        const String& targetName = a[0].string();
        alwaysAssertM(notNull(scene) && notNull(entity), "entity() Track requires non-nullptr Scene and Entity");
        debugAssert(targetName != "");

        CFrame cframe;
        if (a.size() > 1) { cframe = CFrame(a[1]); }
        return EntityTrack::create(entity, targetName, scene, cframe);

    } else if (a.name() == "transform") {
        
        return TransformTrack::create(create(entity, scene, a[0], variableTable), 
                                      create(entity, scene, a[1], variableTable));
        
    } else if (a.name() == "follow") {

        a.verify(false, "follow Tracks are unimplemented");
        return nullptr;
        // return shared_ptr<Entity::Track>(new TransformTrack(create(a[0]), create(a[1])));

    } else if (a.name() == "orbit") {

        return OrbitTrack::create(a[0], a[1]);

    } else if (a.name() == "combine") {

        return CombineTrack::create(create(entity, scene, a[0], variableTable), 
                                    create(entity, scene, a[1], variableTable));

    } else if (a.name() == "lookAt") {

        return LookAtTrack::create(create(entity, scene, a[0], variableTable), 
                                   create(entity, scene, a[1], variableTable), (a.size() > 2) ? Vector3(a[2]) : Vector3::unitY());

    } else if (a.name() == "timeShift") {

        const shared_ptr<Track>& p = create(entity, scene, a[0], variableTable);
        a.verify(notNull(dynamic_pointer_cast<SplineTrack>(p)) || notNull(dynamic_pointer_cast<OrbitTrack>(p)), "timeShift() requires a PhysicsFrameSpline or orbit");
        return TimeShiftTrack::create(p, float(a[1].number()));

    } else if (a.name() == "with") {

        // Create a new variable table and recurse
        VariableTable extendedTable(&variableTable);

        const Any& vars = a[0];
        for (Table<String, Any>::Iterator it = vars.table().begin(); it.isValid(); ++it) {
            // Note that if Any allowed iteration through its table in definition order, then
            // we could implement Scheme LET* instead of LET here.
            extendedTable.set(it->key, create(entity, scene, it->value, variableTable));
        }

        return create(entity, scene, a[1], extendedTable);

    } else {

        // Some failure
        a.verify(false, "Unrecognized Entity::Track type");
        return shared_ptr<Entity::Track>();

    }
}


}
