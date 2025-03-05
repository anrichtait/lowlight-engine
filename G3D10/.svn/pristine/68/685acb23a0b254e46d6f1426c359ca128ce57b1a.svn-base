/**
  \file G3D-app.lib/include/G3D-app/PointModel.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once

#include "G3D-base/platform.h"
#include "G3D-app/Model.h"
#include "G3D-base/AABox.h"

namespace G3D {
class PointSurface;

/**
  \sa PointSurface 
  */
class PointModel : public Model {
public:

    class Specification {
    public:

        /** Options for the .xyz file format */
        class XYZOptions {
        public:
            /** True if the first two numbers */
            bool                hasLatLong;
            bool                hasIR;

            /** If true, override the other options with ones determined by examining the first non-comment line of the file.
                Default = true*/
            bool                autodetect;

            XYZOptions() : hasLatLong(false), hasIR(false), autodetect(true) {}
        } xyzOptions;

        String              filename;
        bool                center;
        Matrix4             transform;
        float               scale;
        bool                renderAsDisk = true;
        ImageFormat::ColorSpace sourceColorSpace;
        Specification(const String& filename = "") : filename(filename), center(true), 
            transform(1,  0,  0,  0,
                      0,  0,  1,  0,
                      0, -1,  0,  0,
                      0,  0,  0,  1),
            sourceColorSpace(ImageFormat::COLOR_SPACE_SRGB) {}

        Specification(const Any& a);
        Any toAny() const;
    };

protected:

    friend class PointSurface;

    static const int CURRENT_CACHE_FORMAT = 3;

    /** A single Model stores multiple PointArrays so that they can become different 
        Surfaces and culled (or possibly in the future, rigid-body animated) */
    class PointArray : public ReferenceCountedObject {
    public:
        Array<Point3>       cpuPosition;

        /** sRGBA8 */
        Array<Color4unorm8> cpuRadiance;

        AttributeArray      gpuPosition;
 
        /** sRGBA8 */
        AttributeArray      gpuRadiance;

        AABox               boxBounds;
        Sphere              sphereBounds;

        void copyToGPU();
        void computeBounds();
        /** Only call during loading */
        void addPoint(const Point3& position, const Color4unorm8 radiance);

        /** Called after loading by several of the loaders on m_pointArrayArray[0]. */;
        void centerPoints();

        /** Randomizes the order of elements of cpuPosition and cpuRadiance */
        void randomize();

        int64 size() const {
            return cpuPosition.size();
        }
    };

    String              m_name;

    bool                m_renderAsDisk;

    /** Meters */
    float               m_pointRadius;

    int64               m_numPoints;

    AABox               m_boxBounds;

    Array<shared_ptr<PointArray>> m_pointArrayArray;

    void load(const Specification& spec);
    void loadPLY(const Specification& spec);
    void loadXYZ(const Specification& spec);
    void loadVOX(const Specification& spec);

    /** Returns false if the cache load fails */
    bool loadCache(const String& filename);

    /** Only call during loading. Write to pointArrayArray[0] */
    void addPoint(const Point3& position, const Color4unorm8 radiance);

    PointModel(const String& name) : m_name(name) {}

    /** Divides pointArrayArray[0] into multiple arrays */
    void buildGrid(const Vector3& cellSize);

    /** Called after loading to upload all point arrays to the GPU */
    void copyToGPU();

    /** Called after loading to compute bounds on all point arrays */
    void computeBounds();

    void saveCache(const String& filename) const;

    /** Given a full path (in either Windows or Unix format), mangles it into a name that can be used
        as a legal filename. e.g.:
        
        "C:/foo/bar/baz_bat/file.ply" ==> "C_c_sfoo_bbar_sbaz_ubat_sfile_pply"

        By the transformation:
        ':' ==> '_c'
        '/' ==> '_s'
        '\\' ==> '_b'
        '.' ==> '_p'
        '?' ==> '_q'
        '*' ==> '_a'
        '_' ==> '_u'
      */
    static String manglePathToFilename(const String& filename);

    /** 
      Returns a binary cache file for \a filename if it exists and is not out of date.
      Otherwise returns "".
    */
    static String makeCacheFilename(const String& filename);

public:


    void pose
    (Array<shared_ptr<Surface> >&       surfaceArray,
     const CFrame&                      rootFrame, 
     const CFrame&                      prevFrame,
     const shared_ptr<Entity>&          entity,
     const Model::Pose*                 pose,
     const Model::Pose*                 prevPose,
     const Surface::ExpressiveLightScatteringProperties& e) override;

    bool renderAsDisk() const {
        return m_renderAsDisk;
    }

    bool intersect
    (const Ray&                      ray, 
     const CoordinateFrame&          cframe, 
     float&                          maxDistance, 
     Model::HitInfo&                 info        = Model::HitInfo::ignore,
     const Entity*                   entity      = nullptr,
     const Model::Pose*              pose        = nullptr) const override;

    virtual const String& className() const override {
        static const String m = "PointModel";
        return m;
    }

    virtual const String& name() const override {
        return m_name;
    }

    /** \param filename .points or .ply file, or a directory containing multiple .ply files */
    static shared_ptr<PointModel> create(const String& name, const Specification& specification);


    static lazy_ptr<Model> lazyCreate(const String& name, const Any& a);
    static lazy_ptr<Model> lazyCreate(const PointModel::Specification& specification, const String& name);


    /** in meters */
    float pointRadius() const {
        return m_pointRadius;
    }

    void setPointRadius(float r) {
        m_pointRadius = r;
    }

    int64 numPoints() const {
        return m_numPoints;
    }

    const AABox& boxBounds() const {
        return m_boxBounds;
    }

    void pose(Array<shared_ptr<Surface>>& surfaceArray, const shared_ptr<Entity>& entity = shared_ptr<Entity>()) const;
};

}
