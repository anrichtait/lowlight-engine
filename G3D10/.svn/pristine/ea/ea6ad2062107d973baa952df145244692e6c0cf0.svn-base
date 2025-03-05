/**
  \file G3D-app.lib/include/G3D-app/FontModel.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/


#pragma once

#define GLG3D_FontModel_h

#include "G3D-base/platform.h"
#include "G3D-app/Model.h"
#include "G3D-app/Surface.h"
#include "G3D-app/GFont.h"
#include "G3D-app/Entity.h"
namespace G3D {

class TextSurface;

/**
   \sa TextSurface
*/
class FontModel : public Model {
public:

    friend class TextSurface;
    
    class Pose : public Model::Pose {
    public:     
        String  text;
        Color3  color;
        float   size;
        
        Surface::ExpressiveLightScatteringProperties expressiveLightScatteringProperties;
        Pose() {}
        virtual bool differentBounds(const shared_ptr<Model::Pose>& other) const override;
        virtual ~Pose() {}
        shared_ptr<Model::Pose> clone() const override {
            const shared_ptr<Pose>& p = createShared<Pose>();
            *p = *this;
            return p;
        }
    };
    
    class Specification {
    public:

        String  filename;
        String  text = "Hello World!";
        Color3  color = Color3::white();
        float   scale = 0.3f;

        Specification(const String& filename = "") : filename(filename) {}

        Specification(const Any& a);

        Any toAny() const;
    };

protected:

    /** We keep the font on the Model because we do not need to change it dynamically. */
    shared_ptr<GFont>               font;
    shared_ptr<FontModel::Pose>     m_pose;

    String                          m_name;

    FontModel(const String& name) : m_name(name) {}

    //void pose(Array<shared_ptr<Surface>>& surfaceArray, const shared_ptr<Entity>& entity = shared_ptr<Entity>()) const;

public:

    shared_ptr<FontModel::Pose> modelPose() {
        return m_pose;
    }

    virtual void pose(Array<shared_ptr<Surface> >& surfaceArray, const CFrame& rootFrame, const CFrame& prevFrame, const shared_ptr<Entity>& entity, const Model::Pose* pose, const Model::Pose* prevPose, const Surface::ExpressiveLightScatteringProperties& e) override;

    virtual const String& className() const override {
        static const String m = "FontModel";
        return m;
    }

    virtual const String& name() const override {
        return m_name;
    }

    static shared_ptr<FontModel> create(const FontModel::Specification& specification, const String& name);


    static lazy_ptr<Model> lazyCreate(const String& name, const Any& a);
    static lazy_ptr<Model> lazyCreate(const FontModel::Specification& specification, const String& name);

    bool intersect
    (const Ray&                      R,
     const CoordinateFrame&          cframe,
     float&                          maxDistance,
     Model::HitInfo&                 info,
     const Entity*                   entity,
     const Model::Pose*              pose) const override;
}; // class FontModel
    
} // namespace G3D
