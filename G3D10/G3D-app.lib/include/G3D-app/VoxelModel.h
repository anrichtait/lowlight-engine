/**
  \file G3D-app.lib/include/G3D-app/VoxelModel.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/


#pragma once
#include "G3D-app/Model.h"
#include "G3D-base/platform.h"
#include "G3D-base/AABox.h"
#include "G3D-base/ParseSchematic.h"
namespace G3D {
class VoxelSurface;

/**
  \sa VoxelSurface 
  */
class VoxelModel : public Model {
public:
    friend class VoxelSurface;

    class Specification {
    public:

        String              filename;
        bool                center;
        Matrix4             transform;
        float               scale = 1;
        bool                removeInternalVoxels = true;
        int                 duplicate = 1;
        bool                treatBorderAsOpaque = false;

        String              origin = "CENTER";

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

    friend class VoxelSurface;

    static const int CURRENT_CACHE_FORMAT = 3;

    /** Integers in voxel space */
    Array<Point3int16>  m_cpuPosition;

    /** sRGBA8 */
    Array<Color4unorm8> m_cpuColor;

    /** Integers TODO: Store in RGB16I */
    AttributeArray      m_gpuPosition;
 
    /** sRGBA8 */
    AttributeArray      m_gpuColor;

    /** In object space (not voxel space) */
    AABox               m_boxBounds;

    /** In object space (not voxel space) */
    Sphere              m_sphereBounds;
    
    String              m_name;

    /** Meters */
    float               m_voxelRadius;
    
    void copyToGPU();
        
    void computeBounds();

    /** Only call during loading. Called by addVoxels */
    void addVoxel(const Point3int16 position, const Color4unorm8 color);

    void centerModel(const Point3int16 originOffset);        

    void load(const Specification& spec);
    void loadVOX(const Specification& spec);

    void loadSchematic(const Specification& specification);

    void addVoxels(const ParseSchematic::ColorVoxels& voxels, Point3int32 minBound, Point3int32 maxBound, const Specification& specification);
    
    VoxelModel(const String& name) : m_name(name) {}
    
public:

    void pose(Array<shared_ptr<Surface> >& surfaceArray, const CFrame& rootFrame, const CFrame& prevFrame, const shared_ptr<Entity>& entity, const Model::Pose* pose, const Model::Pose* prevPose,
     const Surface::ExpressiveLightScatteringProperties& e) override;

    bool intersect
       (const Ray&                      R, 
        const CoordinateFrame&          cframe, 
        float&                          maxDistance, 
        Model::HitInfo&                 info = Model::HitInfo::ignore,
        const Entity*                   entity = nullptr,
        const Model::Pose*              pose = nullptr) const override;

    virtual const String& className() const override {
        static const String m = "VoxelModel";
        return m;
    }

    virtual const String& name() const override {
        return m_name;
    }

    /** \param filename .points or .ply file, or a directory containing multiple .ply files */
    static shared_ptr<VoxelModel> create(const String& name, const Specification& specification);


    static lazy_ptr<Model> lazyCreate(const String& name, const Any& a);
    static lazy_ptr<Model> lazyCreate(const VoxelModel::Specification& specification, const String& name);

    /** in meters */
    float voxelRadius() const {
        return m_voxelRadius;
    }
    
    int64 numVoxels() const {
        return m_cpuPosition.size();
    }

    void pose(Array<shared_ptr<Surface>>& surfaceArray, const shared_ptr<Entity>& entity = nullptr) const;

    /** For debugging */
    static int64 voxelsRendered;

};
}//namespace G3D
