/**
  \file G3D-app.lib/source/FontModel.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/


#include "G3D-app/FontModel.h"
#include "G3D-app/TextSurface.h"

namespace G3D {


bool FontModel::Pose::differentBounds(const shared_ptr<Model::Pose>& _other) const {
  const shared_ptr<FontModel::Pose>& other = dynamic_pointer_cast<FontModel::Pose>(_other);

    if (isNull(other)) {
        return true;
    }

    return ((text != other->text) || (size != other->size));
}


shared_ptr<FontModel> FontModel::create(const FontModel::Specification& specification, const String& name) {
    const shared_ptr<FontModel>& v = createShared<FontModel>(name);

    // Initialize font, size, color (for now, these are on the model, but they can be easily moved to the pose).
    v->font = GFont::fromFile(specification.filename);
    v->m_pose = std::make_shared<FontModel::Pose>();
    v->m_pose->text = specification.text;
    v->m_pose->color = specification.color;
    v->m_pose->size = specification.scale;

    return v;
}


Any FontModel::Specification::toAny() const {
    Any a(Any::TABLE, "FontModel::Specification");
    a["filename"]   = filename;
    a["text"]       = text;
    a["color"]      = color;
    a["scale"]      = scale;
    return a;
}

FontModel::Specification::Specification(const Any& a) {
    *this = Specification();

    if (a.type() == Any::STRING) {
        try {
            filename = a.resolveStringAsFilename();
        }
        catch (const FileNotFound& e) {
            throw ParseError(a.source().filename, a.source().line, e.message);
        }
    }
    else {

        AnyTableReader r(a);
        Any f;
        if (!r.getIfPresent("filename", f)) {
            a.verify(false, "Expected a filename field in FontModel::Specification");
        }
        f.verifyType(Any::STRING);
        try {
            filename = f.resolveStringAsFilename();
        }
        catch (const FileNotFound& e) {
            throw ParseError(f.source().filename, f.source().line, e.message);
        }

        r.getIfPresent("text", text);
        r.getIfPresent("color", color);
        r.getIfPresent("scale", scale);
        r.verifyDone();
    }
}


lazy_ptr<Model> FontModel::lazyCreate(const String& name, const Any& a) {
    return lazyCreate(a, name);
}

lazy_ptr<Model> FontModel::lazyCreate(const FontModel::Specification& specification, const String& name) {
    return lazy_ptr<Model>([specification, name] { return FontModel::create(specification, name); }); //Order might need to be switched to match ArticulatedModel
}

//void FontModel::pose(Array<shared_ptr<Surface>>& surfaceArray, const shared_ptr<Entity>& entity) const {
//    const shared_ptr<FontModel>& me = dynamic_pointer_cast<FontModel>(const_cast<FontModel*>(this)->shared_from_this());
//
//    surfaceArray.append(TextSurface::create(name(), 
//                        notNull(entity) ? entity->frame() : CFrame(),
//                        notNull(entity) ? entity->previousFrame() : CFrame(),
//                        me, 
//                        entity, 
//                        Surface::ExpressiveLightScatteringProperties()));
//}

void FontModel::pose(Array<shared_ptr<Surface> >& surfaceArray, const CFrame& rootFrame, const CFrame& prevFrame, const shared_ptr<Entity>& entity, const Model::Pose* pose, const Model::Pose* prevPose, const Surface::ExpressiveLightScatteringProperties& e) {
    const shared_ptr<FontModel>& me = dynamic_pointer_cast<FontModel>(const_cast<FontModel*>(this)->shared_from_this());

    surfaceArray.append(TextSurface::create(name(),
        notNull(entity) ? entity->frame() : rootFrame,
        notNull(entity) ? entity->previousFrame() : prevFrame,
        me,
        entity,
        Surface::ExpressiveLightScatteringProperties()));
}


bool FontModel::intersect
   (const Ray&                      R,
    const CoordinateFrame&          cframe,
    float&                          maxDistance,
    Model::HitInfo&                 info,
    const Entity*                   entity,
    const Model::Pose*              pose) const {

    return entity->intersectBounds(R, maxDistance, info);
}
}// namespace G3D
