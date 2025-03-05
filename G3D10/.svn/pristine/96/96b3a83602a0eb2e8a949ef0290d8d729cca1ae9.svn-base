/**
  \file G3D-app.lib/source/VoxelModel.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/


#include "G3D-app/VoxelModel.h"
#include "G3D-app/VoxelSurface.h"

#include "G3D-app/Entity.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/ParseVOX.h"
#include "G3D-base/Ray.h"

namespace G3D {
int64 VoxelModel::voxelsRendered = 0;

shared_ptr<VoxelModel> VoxelModel::create(const String& name, const Specification& spec) {
    const shared_ptr<VoxelModel>& v = createShared<VoxelModel>(name);
    v->load(spec);
    return v;
}


Any VoxelModel::Specification::toAny() const {
    Any a(Any::TABLE, "ArticulatedModel::Specification");
    a["filename"]                   = filename;
    a["scale"]                      = scale;
    a["removeInternalVoxels"]       = removeInternalVoxels;
    a["duplicate"]                  = duplicate;
    a["treatBorderAsOpaque"]        = treatBorderAsOpaque;
    a["origin"]                     = origin;
    return a;
}


VoxelModel::Specification::Specification(const Any& a) {
    *this = Specification();

    if (a.type() == Any::STRING) {
        try {
            filename = a.resolveStringAsFilename();
        } catch (const FileNotFound& e) {
            throw ParseError(a.source().filename, a.source().line, e.message);
        }
    } else {

        AnyTableReader r(a);
        Any f;
        if (! r.getIfPresent("filename", f)) {
            a.verify(false, "Expected a filename field in VoxelModel::Specification");
        }
        f.verifyType(Any::STRING);
        try {
            filename = f.resolveStringAsFilename();   
        } catch (const FileNotFound& e) {
            throw ParseError(f.source().filename, f.source().line, e.message);
        }

        r.getIfPresent("scale",                         scale);
        r.getIfPresent("removeInternalVoxels",          removeInternalVoxels);
        r.getIfPresent("duplicate",                     duplicate);
        r.getIfPresent("treatBorderAsOpaque",           treatBorderAsOpaque);
        r.getIfPresent("origin",                        origin);
        r.verifyDone();
    }
}


lazy_ptr<Model> VoxelModel::lazyCreate(const String& name, const Any& a) {
    return lazyCreate(a, name);
}


lazy_ptr<Model> VoxelModel::lazyCreate(const VoxelModel::Specification& specification, const String& name) {
    return lazy_ptr<Model>([specification, name]{ return VoxelModel::create(name, specification); }); //the order should be switched to match Articulated Model
}


void VoxelModel::copyToGPU() {
    alwaysAssertM(m_cpuColor.size() == m_cpuPosition.size(), "Attribute arrays must be the same size");
    const int64 N = m_cpuPosition.size();

    // Throw away old values to free GPU memory
    m_gpuPosition = AttributeArray();
    m_gpuColor    = AttributeArray();

    const shared_ptr<VertexBuffer>& buffer = VertexBuffer::create(N * (sizeof(Point3int16) + sizeof(Color4unorm8)) + 16, VertexBuffer::WRITE_ONCE);
    
    m_gpuPosition = AttributeArray(m_cpuPosition, buffer);
    m_gpuColor    = AttributeArray(m_cpuColor, buffer);
}


void VoxelModel::load(const Specification& spec) {

    m_cpuPosition.clear();
    m_cpuColor.clear();
    m_gpuPosition = AttributeArray();
    m_gpuColor    = AttributeArray();

    m_voxelRadius = 0.005f * spec.scale;

    if (endsWith(toLower(spec.filename), ".vox")) {
        loadVOX(spec);
    } else if (endsWith(toLower(spec.filename), ".schematic")) {
        loadSchematic(spec);
    } else {
        alwaysAssertM(false, "Illegal filename");
    }

    computeBounds();
    copyToGPU();
}


void VoxelModel::computeBounds() {
    if (m_cpuPosition.size() == 0) {
        m_boxBounds = AABox();
        m_sphereBounds = Sphere();
        return;
    }

    Point3int16 lo = m_cpuPosition[0];
    Point3int16 hi = lo;

    for (Point3int16 P : m_cpuPosition) {
        lo = lo.min(P);
        hi = hi.max(P);
    }

    m_boxBounds = AABox((Point3(lo) - Point3(0.5f, 0.5f, 0.5f)) * 2.0f * m_voxelRadius, (Point3(hi) + Point3(0.5f, 0.5f, 0.5f)) * 2.0f * m_voxelRadius);
    m_boxBounds.getBounds(m_sphereBounds);
}


void VoxelModel::centerModel(const Point3int16 originOffset) {
    if (m_cpuPosition.size() == 0) { return; }

    Point3 center = Point3(m_cpuPosition[0]);
    for (int i = 1; i < m_cpuPosition.size(); ++i) {
        center += Point3(m_cpuPosition[i]);
    }

    center /= float(m_cpuPosition.size());

    const Point3int16 offset = Point3int16(-center + originOffset);
    for (int i = 0; i < m_cpuPosition.size(); ++i) {
        m_cpuPosition[i] += offset;
    }
}


void VoxelModel::loadSchematic(const Specification& specification) {

    ParseSchematic parseData;
    parseData.parse(specification.filename);
    
    ParseSchematic::ColorVoxels voxels;
    parseData.getSparseVoxelTable(voxels);
    addVoxels(voxels, Point3int32(0,0,0), parseData.size - Vector3int32(1,1,1), specification);
}


void VoxelModel::loadVOX(const Specification& specification) {
 
    ParseVOX parseData;
    {
        BinaryInput bi(specification.filename, G3D_LITTLE_ENDIAN);
        parseData.parse(bi);
    }

    ParseSchematic::ColorVoxels voxels;

    // Keep track of these to determine the size of the model
    // for duplicating and for handling boundary case
    Point3int32 minBound(32767, 32767, 32767), maxBound(-32767, -32767, -32767);

    // Convert from Point3uint8 to Point3int16 indices and swap coordinate system
    for (const ParseVOX::Voxel v : parseData.voxel) { 
        const Point3int16 position(v.position.x, v.position.z, -v.position.y);
        voxels[position] = parseData.palette[v.index];
        minBound = minBound.min(position);
        maxBound = maxBound.max(position);
    }

    addVoxels(voxels, minBound, maxBound, specification);
}


void VoxelModel::addVoxels
   (const ParseSchematic::ColorVoxels&  voxels,
    const Point3int32                   minBound,
    const Point3int32                   maxBound,
    const Specification&                specification) {


    // TODO: Why aren't these offsets one larger?
    const int16 width  = maxBound.x - minBound.x;
    const int16 length = maxBound.z - minBound.z;

    Vector3int16 originOffset(0,0,0);

    if (specification.origin == "BOTTOM"){
        originOffset.y += 1;
    } else if (specification.origin == "CORNER"){
        originOffset += Vector3int16(1,1,1);
    }

    // For each voxel
    for (ParseSchematic::ColorVoxels::ConstIterator it = voxels.begin(); it.isValid(); ++it) {
        const Point3int16 position(it.key());
        debugAssert(position.x <= maxBound.x && position.y <= maxBound.y && position.z <= maxBound.z &&
                    position.x >= minBound.x && position.y >= minBound.y && position.z >= minBound.z);

        if (specification.removeInternalVoxels) {

            // For each cube face of the voxel
            for (int i = 0; i < 6; ++i) {

                // Create the local coordinate system
                const int axis = i >> 1;
                const int16 direction = (i & 1) * 2 - 1;

                // Find the neighbor on this face
                Point3int16 neighborPosition = position;
                neighborPosition[axis] += direction;

                // Check the neighbor, unless it is out of bounds
                bool neighborIsOpaque = specification.treatBorderAsOpaque &&
                        ((neighborPosition.x > maxBound.x) || (neighborPosition.x < minBound.x) ||
                        (neighborPosition.z > maxBound.z) || (neighborPosition.z < minBound.z) ||
                        (neighborPosition.y < minBound.y));

                neighborIsOpaque = neighborIsOpaque || voxels.containsKey(neighborPosition);

                // Look for a side that is not fully enclosed...if we find one,
                // generate the voxel and then exit the face loop
                if (! neighborIsOpaque) {
                    // This voxel can be seen from the neighbor side
                    addVoxel(position, it.value());
                    break;
                }
            } // for each face
        } else { // Don't remove internal voxels
                    addVoxel(position, it.value());
        } // If remove internal voxels
    } // For each voxel

    if (specification.duplicate > 1) {
        const int oldSize = m_cpuPosition.size();
        m_cpuPosition.resize(oldSize * specification.duplicate * specification.duplicate);
        m_cpuColor.resize(m_cpuPosition.size());

        for (int u = 0; u < specification.duplicate; ++u) {
            for (int v = 0; v < specification.duplicate; ++v) {
                const Point3int16 offset(u * width, 0, v * length);
                const int indexOffset = (specification.duplicate * u + v) * oldSize;
                runConcurrently(0, oldSize, [&](int i) {
                    const int j = i + indexOffset; 
                    m_cpuPosition[j] = m_cpuPosition[i] + offset;
                    m_cpuColor[j]    = m_cpuColor[i];
                });
            } // v
        } // u
    }
    debugPrintf("Loaded a %d visible voxel model (orginally %d voxels)\n", m_cpuPosition.size(),
        square(specification.duplicate) * voxels.size()); 

    // Ensure that positions are uncorrelated with vertex index. This also
    // stabilizes performance independent of viewing direction.
    m_cpuPosition.randomize(m_cpuColor, Random::threadCommon());
    centerModel(originOffset);

}


void VoxelModel::addVoxel(const Point3int16 position, const Color4unorm8 radiance) {
    m_cpuPosition.append(position);
    m_cpuColor.append(radiance);
}


bool VoxelModel::intersect
   (const Ray&                      R, 
    const CoordinateFrame&          cframe, 
    float&                          maxDistance, 
    Model::HitInfo&                 info,
    const Entity*                   entity,
    const Model::Pose*              pose) const {       


    return entity->intersectBounds(R, maxDistance);
}


void VoxelModel::pose(Array<shared_ptr<Surface> >& surfaceArray, const CFrame& rootFrame, const CFrame& prevFrame, const shared_ptr<Entity>& entity, const Model::Pose* pose, const Model::Pose* prevPose,
         const Surface::ExpressiveLightScatteringProperties& expressiveLightScatteringProperties) {
    VoxelModel::pose(surfaceArray, entity);
}


void VoxelModel::pose(Array<shared_ptr<Surface>>& surfaceArray, const shared_ptr<Entity>& entity) const {
    const shared_ptr<VoxelModel>& me = dynamic_pointer_cast<VoxelModel>(const_cast<VoxelModel*>(this)->shared_from_this());
    
    surfaceArray.append(VoxelSurface::create(name(), notNull(entity) ? entity->frame() : CFrame(),
        notNull(entity) ? entity->previousFrame() : CFrame(),
        me, entity, Surface::ExpressiveLightScatteringProperties()));
}
}// namespace G3D
