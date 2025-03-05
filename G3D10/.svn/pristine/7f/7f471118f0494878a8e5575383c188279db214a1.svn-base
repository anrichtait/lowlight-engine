/**
  \file G3D-app.lib/source/ArticulatedModel.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/ArticulatedModel.h"
#include "G3D-base/Ray.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/Stopwatch.h"
#include "G3D-app/GApp.h"

namespace G3D {

const String& ArticulatedModel::className() const {
    static String s("ArticulatedModel");
    return s;
}


String ArticulatedModel::resolveRelativeFilename(const String& filename, const String& basePath) {
    if (filename.empty()) {
        return filename;
    } else {
        return FileSystem::resolve(filename, basePath);
    }
}


Table<ArticulatedModel::Specification, shared_ptr<ArticulatedModel> > s_cache;

void ArticulatedModel::clearCache() {
    s_cache.clear();
}


shared_ptr<ArticulatedModel> ArticulatedModel::loadArticulatedModel(const ArticulatedModel::Specification& specification, const String& n) {
    const shared_ptr<ArticulatedModel>& a = createShared<ArticulatedModel>();

    if (n.empty()) {
        a->m_name = FilePath::base(specification.filename);
    }
    
    a->load(specification);

    if (! n.empty()) {
        a->m_name = n;
    }
    
    return a;
}


lazy_ptr<Model> ArticulatedModel::lazyCreate(const String& name, const Any& a) {
    return lazyCreate(Specification(a), name);
}


lazy_ptr<Model> ArticulatedModel::lazyCreate(const ArticulatedModel::Specification& specification, const String& name) {
    return lazy_ptr<Model>([specification, name]{ return ArticulatedModel::create(specification, name); });
}


shared_ptr<ArticulatedModel> ArticulatedModel::create(const ArticulatedModel::Specification& specification, const String& n) {
    if (specification.cachable) {
        bool created = false;
        shared_ptr<ArticulatedModel>& a = s_cache.getCreate(specification, created);
        if (created) {
            a = loadArticulatedModel(specification, n);
        }
        return a;
    } else {
        return loadArticulatedModel(specification, n);
    }
}


shared_ptr<ArticulatedModel> ArticulatedModel::createEmpty(const String& n) {
    const shared_ptr<ArticulatedModel>& a = createShared<ArticulatedModel>();
    a->m_name = n;
    return a;
}


void ArticulatedModel::forEachPart(PartCallback& callback, Part* part, const CFrame& parentFrame, const Pose& pose, const int treeDepth) {
    // Net transformation from part to world space
    const CFrame& net = parentFrame * part->cframe * pose.frame(part->name);

    // Process all children
    for (int c = 0; c < part->m_children.size(); ++c) {
        forEachPart(callback, part->m_children[c], net, pose, treeDepth + 1);
    }

    // Invoke the callback on this part
    callback(part, net, dynamic_pointer_cast<ArticulatedModel>(shared_from_this()), treeDepth);
}


void ArticulatedModel::forEachPart(PartCallback& callback, const CFrame& cframe, const Pose& pose) {
    for (int p = 0; p < m_rootArray.size(); ++p) {
        forEachPart(callback, m_rootArray[p], cframe, pose, 0);
    }
}


ArticulatedModel::Mesh* ArticulatedModel::addMesh(const String& name, Part* part, Geometry* geom) {
    alwaysAssertM(notNull(geom), "Cannot add a mesh with null geometry");
    Mesh* m = new Mesh(name, part, geom, getID());
    m_meshArray.append(m);
    return m;
}


ArticulatedModel::Part* ArticulatedModel::addPart(const String& name, Part* parent) {
    m_partArray.append(new Part(name, parent, getID()));
    if (isNull(parent)) {
        m_rootArray.append(m_partArray.last());
    } else {
        parent->m_children.append(m_partArray.last());
    }

    return m_partArray.last();
}


ArticulatedModel::Geometry* ArticulatedModel::addGeometry(const String& name) {
    Geometry* g = new Geometry(name);
    m_geometryArray.append(g);
    return m_geometryArray.last();
}


ArticulatedModel::Mesh* ArticulatedModel::mesh(int ID) {
    // Exhaustively cycle through all meshes
    for (int m = 0; m < m_meshArray.size(); ++m) {
        if (ID == m_meshArray[m]->uniqueID) {
            return m_meshArray[m];
        }
    }
    
    return nullptr;
}


ArticulatedModel::Mesh* ArticulatedModel::mesh(const String& meshName) {

    // Exhaustively cycle through all meshes
    for (int m = 0; m < m_meshArray.size(); ++m) {
        const String& currentName = m_meshArray[m]->name;
        // Strip the prefix added to the mesh name ("file: "
        // if it came from a texture)
        const size_t prefixEnd = currentName.find_last_of(" ");
        const String& currentNameNoPrefix = currentName.substr(prefixEnd + 1, currentName.length() - prefixEnd);
        if (currentName == meshName || currentNameNoPrefix == meshName) {
            return m_meshArray[m];
        }
    }
    
    return nullptr;
}


ArticulatedModel::Geometry* ArticulatedModel::geometry(const String& geomName) {

    // Exhaustively cycle through all meshes
    for (int g = 0; g < m_geometryArray.size(); ++g) {
        if (m_geometryArray[g]->name == geomName) {
            return m_geometryArray[g];
        }
    }
    
    return nullptr;
}


ArticulatedModel::Part* ArticulatedModel::part(const Instruction::Identifier& partIdent) {
    return part(partIdent.name);
}


ArticulatedModel::Mesh* ArticulatedModel::mesh(const Instruction::Identifier& meshIdent) {
    return mesh(meshIdent.name);
}


ArticulatedModel::Geometry* ArticulatedModel::geometry(const Instruction::Identifier& geomIdent) {
    return geometry(geomIdent.name);
}


void ArticulatedModel::getIdentifiedMeshes(const Instruction::Identifier& meshIdent, Array<Mesh*>& identifiedMeshes) {
    if (meshIdent.isAll()) {
        identifiedMeshes.appendPOD(m_meshArray);
    } else if (! meshIdent.name.empty()) {
        Mesh* identifiedMesh = mesh(meshIdent);
        alwaysAssertM(notNull(identifiedMesh), format("Tried to access nonexistent mesh %s", meshIdent.name.c_str()));
        identifiedMeshes.append(identifiedMesh);
    } else {
        alwaysAssertM(false, "Only named meshes or <all> currently can be specified for getIdentifiedMeshes");
    }
}


void ArticulatedModel::getIdentifiedGeometry(const Instruction::Identifier& geomIdent, Array<Geometry*>& identifiedGeometry) {
    if (geomIdent.isAll()) {
        identifiedGeometry.appendPOD(m_geometryArray);
    } else if (! geomIdent.name.empty()){
        identifiedGeometry.append(geometry(geomIdent));
    } else {
        alwaysAssertM(false, "Only named geometry or <all> currently can be specified for identifiedGeometry");
    }
}


ArticulatedModel::Part* ArticulatedModel::part(const String& partName) {
    // Exhaustively cycle through all parts
    for (int p = 0; p < m_partArray.size(); ++p) {
        const String& n = m_partArray[p]->name;
        if (n == partName) {
            return m_partArray[p];
        }
    }
    return nullptr;
}

// Set to true to print timings when loading
const bool timeArticulatedModelLoad = false;

void ArticulatedModel::load(const Specification& specification) {
    m_sourceSpecification = specification;
    ContinuousStopwatch timer;

    timer.setEnabled(timeArticulatedModelLoad);
    m_sourceSpecification = specification;
    
    const String& ext = toLower(FilePath::ext(specification.filename));

    if (ext == "obj") {
        loadOBJ(specification);
    } else if (ext == "ifs") {
        loadIFS(specification);
    } else if (ext == "ply2") {
        loadPLY2(specification);
    } else if (ext == "ply") {
        loadPLY(specification);
    } else if (ext == "off") {
        loadOFF(specification);
    } else if (ext == "vox") {
        loadVOX(specification);
    } else if (ext == "3ds") {
        load3DS(specification);
    } else if (ext == "bsp") {
        loadBSP(specification);
    } else if ((ext == "stl") || (ext == "stla")) {
        loadSTL(specification);
    } else if ((ext == "dae") || (ext == "fbx") || (ext == "lwo") || (ext == "ase") || (ext == "glb") || (ext == "gltf")) {
        loadASSIMP(specification);
    } else if (ext == "hair") {
        loadHAIR(specification);
    } else if (ext == "schematic") {
        loadSchematic(specification);
    } else {
        loadHeightfield(specification);
    }
    timer.printElapsedTime("import file");
    
    if (((specification.meshMergeOpaqueClusterRadius != 0.0f) ||
        (specification.meshMergeTransmissiveClusterRadius != 0.0f)) &&
        (m_meshArray.length() > 1) &&

        // OBJ files with infinite merge radius are pre-merged
        !((ext == "obj") && (specification.meshMergeOpaqueClusterRadius == finf()) && (specification.meshMergeTransmissiveClusterRadius == finf())) &&

        // Hair models have all polygons so close that nothing useful will happen during merging
        (ext != "hair") &&
        
        // voxel models don't need to be merged (TODO: why not?)
        (ext != "vox")) {
        MeshMergeCallback merge(specification.meshMergeOpaqueClusterRadius, specification.meshMergeTransmissiveClusterRadius);
        forEachPart(merge);
        timer.printElapsedTime("merge geometry");
    }
    
    // If this model is very large, compact the vertex arrays to save RAM
    // during the post-processing step
    maybeCompactArrays();
    
    // Perform operations as demanded by the specification
    if (specification.scale != 1.0f) {
        scaleWholeModel(specification.scale);
    }
    preprocess(specification.preprocess);
    timer.printElapsedTime("preprocess");
    
    // Compute missing elements (normals, tangents) of the part geometry, 
    // perform vertex welding, and recompute bounds.
    if ((ext != "hair") && (ext != "vox")) {
        cleanGeometry(specification.cleanGeometrySettings);
    }

    maybeCompactArrays();
    computeBounds();
    
    timer.printElapsedTime("cleanGeometry");
}


void ArticulatedModel::clearGPUArrays() {
    for (int g = 0; g < m_geometryArray.size(); ++g) {
        m_geometryArray[g]->clearAttributeArrays();
    }

    for (int m = 0; m < m_meshArray.size(); ++m) {
        m_meshArray[m]->clearIndexStream();
    }
}


float ArticulatedModel::anyToMeshMergeRadius(const Any& a) {
    if (a.type() == Any::NUMBER) {
        return float(a.number());
    } else if (a.type() == Any::STRING) {
        if (a.string() == "AUTO") {
            return -finf();
        } else if (a.string() == "NONE") {
            return 0.0f;
        } else if (a.string() == "ALL") {
            return finf();
        } else {
            a.verify(false, "Unrecognized mesh merge radius named constant");
            return finf();
        }
    } else {
        a.verify(false, "Unrecognized mesh merge radius value");
        return finf();
    }
}


Any ArticulatedModel::meshMergeRadiusToAny(const float r) {
    if (r == 0.0f) {
        return Any("NONE");
    } else if (r < -1.0f) {
        return Any("AUTO");
    } else if (r == finf()) {
        return Any("ALL");
    } else {
        return Any(r);
    }
}


void ArticulatedModel::maybeCompactArrays() {
    int numVertices     = 0;
    int numIndices      = 0;
    int numTexCoord1    = 0;
    int numVertexColors = 0;
    // TODO: Take into account bones
    for (int g = 0; g < m_geometryArray.size(); ++g) {
        Geometry* geom = m_geometryArray[g];
        numVertices += geom->cpuVertexArray.size();
        if (geom->cpuVertexArray.hasTexCoord1) {
            numTexCoord1 += geom->cpuVertexArray.texCoord1.size();
        }
        if (geom->cpuVertexArray.hasVertexColors) {
            numVertexColors += geom->cpuVertexArray.vertexColors.size();
        }
    }

    for (int m = 0; m < m_meshArray.size(); ++m) {
        numIndices += m_meshArray[m]->cpuIndexArray.size();
    }
    
    size_t vertexBytes      = sizeof(CPUVertexArray::Vertex) * numVertices;
    size_t indexBytes       = sizeof(int) * numIndices;
    size_t texCoord1Bytes   = sizeof(Point2unorm16) * numTexCoord1;
    size_t vertexColorBytes = sizeof(Color4) * numVertexColors;

    if (vertexBytes + indexBytes + texCoord1Bytes + vertexColorBytes > 5000000) {
        // There's a lot of data in this model: compact it

        for (int g = 0; g < m_geometryArray.size(); ++g) {
            Geometry* geom = m_geometryArray[g];
            geom->cpuVertexArray.vertex.trimToSize();
            if (geom->cpuVertexArray.hasTexCoord1) {
                geom->cpuVertexArray.texCoord1.trimToSize();
            }
            if (geom->cpuVertexArray.hasVertexColors) {
                geom->cpuVertexArray.vertexColors.trimToSize();
            }
        }

        for (int m = 0; m < m_meshArray.size(); ++m) {
            m_meshArray[m]->cpuIndexArray.trimToSize();
        }
    }
}


/** Used by ArticulatedModel::intersect */
class AMIntersector : public ArticulatedModel::MeshCallback {
public:
    bool                        hit;

private:
    const Ray&                  m_wsR;
    float&                      m_maxDistance;
    Model::HitInfo&             m_info;
    Table<ArticulatedModel::Part*, CFrame> m_cframeTable;
    const shared_ptr<Entity>& m_entity; 

public:

    AMIntersector(const Ray& wsR, float& maxDistance, Model::HitInfo& information, Table<ArticulatedModel::Part*, CFrame>& cframeTable, const shared_ptr<Entity>& entityset) :
        hit(false), 
        m_wsR(wsR), 
        m_maxDistance(maxDistance), 
        m_info(information), 
        m_cframeTable(cframeTable),
        m_entity(entityset){
    }

    virtual void operator()(shared_ptr<ArticulatedModel> model, ArticulatedModel::Mesh* mesh) override {
        SmallArray<CFrame, 8> jointCFrameArray;
        Vector3 normal;

        AABox boxBounds;
        for (int i = 0; i < mesh->contributingJoints.size(); ++i) {
            const CFrame& jointCFrame = m_cframeTable.get(mesh->contributingJoints[i]) * mesh->contributingJoints[i]->inverseBindPoseTransform;
            jointCFrameArray.append(jointCFrame);
            AABox jointBounds;
            jointCFrame.toWorldSpace(mesh->boxBounds).getBounds(jointBounds);
            boxBounds.merge(jointBounds);
        }
        // Bounding sphere test
        const float testTime = m_wsR.intersectionTime(boxBounds);

        if (testTime >= m_maxDistance) {
            // Could not possibly hit this part's geometry since it doesn't
            // hit the bounds
            return;
        }
        
        bool justHit = false;
        int hitIndex = -1;
        float hitW0 = 0.0f, hitW2 = 0.0f;

        const int numIndices = mesh->cpuIndexArray.size();
        const int* index = mesh->cpuIndexArray.getCArray();
        const Array<CPUVertexArray::Vertex>& vertex = mesh->geometry->cpuVertexArray.vertex;

        if (notNull(mesh->triTree)) {
            // Cast against the tri tree
            const Ray& osRay = jointCFrameArray[0].toObjectSpace(m_wsR);
            TriTree::Hit hitData;
            TriTree::IntersectRayOptions options;
            // We do not copy alpha textures to the CPU
            options = TriTree::DO_NOT_CULL_BACKFACES | TriTree::NO_PARTIAL_COVERAGE_TEST;
            if (mesh->triTree->intersectRay(osRay, hitData, options) && (hitData.distance < m_maxDistance)) {
                hit             = true;
                hitW0           = hitData.u;
                hitW2           = hitData.v;
                hitIndex        = hitData.triIndex * 3;
                m_maxDistance   = hitData.distance;
                int i = hitIndex;
                justHit         = true;

                Point3 p[3];                
                p[0] = vertex[index[i]].position;
                p[1] = vertex[index[i + 1]].position;
                p[2] = vertex[index[i + 2]].position;
                normal      = (p[1] - p[0]).cross(p[2] - p[0]).direction();

                if (hitData.backface) {
                    normal = -normal;
                }
            }

        } else {
        
            alwaysAssertM(mesh->primitive == PrimitiveType::TRIANGLES, 
                            "Only implemented for PrimitiveType::TRIANGLES meshes.");

            const Array<Vector4>& boneWeightArray       = mesh->geometry->cpuVertexArray.boneWeights;
            const Array<Vector4int32>& boneIndexArray   = mesh->geometry->cpuVertexArray.boneIndices;

            Ray intersectingRay;

            // Needed if doing bone animation
            SmallArray<int, 8> contributingIndexArray; 

            if (mesh->contributingJoints.size() == 1) {
                intersectingRay = jointCFrameArray[0].toObjectSpace(m_wsR);
            } else {
                // Transform vertices instead
                intersectingRay = m_wsR; 
                contributingIndexArray.resize(jointCFrameArray.size());
                for (int i = 0; i < contributingIndexArray.size(); ++i) {
                    contributingIndexArray[i] = model->m_boneArray.findIndex(mesh->contributingJoints[i]);
                }
            }

            Array<Point3> triMesh;
            for (int i = 0; i < numIndices; i += 3) {    
            
                Point3 p[3];
                if (mesh->contributingJoints.size() == 1) {
                    p[0] = vertex[index[i]].position;
                    p[1] = vertex[index[i + 1]].position;
                    p[2] = vertex[index[i + 2]].position;
                } else {
                    for (int j = 0; j < 3; ++j) {
                        const int currentIndex          = index[i + j];
                        const Vector4&  boneWeights     = boneWeightArray[currentIndex];
                        const Vector4int32& boneIndices = boneIndexArray[currentIndex];
                        CFrame boneTransform;
                        boneTransform.rotation = Matrix3::diagonal(0.0f,0.0f,0.0f);
                        for (int k = 0; k < 4; ++k) {
                            const int boneIndex = boneIndices[k];
                        
                            for (int b = 0; b < contributingIndexArray.size(); ++b) {
                                if ( contributingIndexArray[b] == boneIndex ) {
                                    boneTransform.rotation      = boneTransform.rotation + (jointCFrameArray[b].rotation * boneWeights[k]);
                                    boneTransform.translation   = boneTransform.translation + (jointCFrameArray[b].translation * boneWeights[k]);
                                }
                            }
                        }
                        p[j] = boneTransform.pointToWorldSpace(vertex[currentIndex].position);
      
                    }
                    triMesh.append(p[0], p[1], p[2]);
                }

                // Barycentric weights
                float w0 = 0, w1 = 0, w2 = 0;
                float testTime = intersectingRay.intersectionTime(p[0], p[1], p[2], w0, w1, w2);

                if (testTime < m_maxDistance) {
                    hit         = true;
                    justHit     = true;
                    m_maxDistance = testTime;
                    normal      = (p[1] - p[0]).cross(p[2] - p[0]).direction();
                    hitIndex    = i;
                    hitW0 = w0; hitW2 = w2;
                } else if (mesh->twoSided) {
                    // Check the backface. We can't possibly hit it unless
                    // the test failed for the front face of this
                    // triangle.
                    testTime = intersectingRay.intersectionTime(p[0], p[2], p[1], w0, w1, w2);
                    
                    if (testTime < m_maxDistance) {
                        hit         = true;
                        justHit     = true;
                        m_maxDistance = testTime;
                        normal      = (p[2] - p[0]).cross(p[1] - p[0]).direction();
                        hitIndex    = i;
                        hitW0 = w0; hitW2 = w2;
                    }
                }

            } // for each triangle
        }

        if (justHit) {
            m_info.set(
                model, 
                m_entity,
                mesh->material,
                mesh->contributingJoints.size() == 1 ? jointCFrameArray[0].normalToWorldSpace(normal) : normal,
                m_wsR.origin() + m_maxDistance * m_wsR.direction(),
                notNull(mesh->logicalPart) ? mesh->logicalPart->name : "",
                mesh->name,
                mesh->uniqueID,
                hitIndex / 3,
                hitW0,
                hitW2);
        }
#   if 0
        if (triMesh.size() > 0) {
            Array<int> indices;
            indices.resize(triMesh.size());
            for (int i = 0; i < triMesh.size(); ++i) {
                indices[i] = i;
            }
            debugDraw(shared_ptr<MeshShape>(new MeshShape(triMesh, indices)), 0.5f);
        }
#   endif
        
    } // operator ()
}; // AMIntersector


bool ArticulatedModel::intersect
(const Ray&                      ray, 
 const CoordinateFrame&          cframe, 
 float&                          maxDistance, 
 Model::HitInfo&                 info, 
 const Entity*                   entity,
 const Model::Pose*              _pose) const {   

    const Pose* __pose = dynamic_cast<const Pose*>(_pose);
    static const Pose defaultPose;
    const Pose& pose = __pose ? *__pose : defaultPose;

    ArticulatedModel* me = const_cast<ArticulatedModel*>(this);

    // TODO: change structure to no longer keep cache of part transforms
    me->computePartTransforms(me->m_partTransformTable, me->m_partTransformTable, cframe, pose, cframe, pose);
    AMIntersector intersectOperation(ray, maxDistance, info, me->m_partTransformTable, 
        entity ? dynamic_pointer_cast<Entity>(const_cast<Entity*>(entity)->shared_from_this()) : nullptr);

    me->forEachMesh(intersectOperation);

    return intersectOperation.hit;
}


void ArticulatedModel::countTrianglesAndVertices(int& tri, int& vert) const {
    tri = 0;
    vert = 0;
    for (int m = 0; m < m_meshArray.size(); ++m) {
        const Mesh* mesh = m_meshArray[m];
        tri += mesh->triangleCount();
    }

    for (int g = 0; g < m_geometryArray.size(); ++g) {
        const Geometry* geom = m_geometryArray[g];
        vert += geom->cpuVertexArray.size();
    }
}


ArticulatedModel::~ArticulatedModel() {
    m_partArray.invokeDeleteOnAllElements();
    m_meshArray.invokeDeleteOnAllElements();
    m_geometryArray.invokeDeleteOnAllElements();
}


void ArticulatedModel::getBoundingBox(AABox& box) {
    Array<shared_ptr<Surface> > arrayModel;
    Animation animation;
    Pose pos;
    if (usesSkeletalAnimation()) {
        Array<String> animationNames;
        getAnimationNames(animationNames);
        // TODO: Add support for selecting animations.
        getAnimation(animationNames[0], animation); 
        animation.getCurrentPose(0.0f, pos);
    } 
    
    pose(arrayModel, CFrame(), CFrame(), nullptr, nullptr, nullptr, Surface::ExpressiveLightScatteringProperties());

    int numFaces = 0;
    int numVertices = 0;
    countTrianglesAndVertices(numFaces, numVertices);
    
    bool overwrite = true;
    
    if (arrayModel.size() > 0) {
        
        for (int x = 0; x < arrayModel.size(); ++x) {       
            
            // Merges the bounding boxes of all the separate parts into the bounding box of the entire object
            AABox temp;
            CFrame cframe;
            arrayModel[x]->getCoordinateFrame(cframe);
            arrayModel[x]->getObjectSpaceBoundingBox(temp);
            Box partBounds = cframe.toWorldSpace(temp);
            
            // Some models have screwed up bounding boxes
            if (partBounds.extent().isFinite()) {
                if (overwrite) {
                    partBounds.getBounds(box);
                    overwrite = false;
                } else {
                    partBounds.getBounds(temp);
                    box.merge(temp);
                }
            }
        }
        
        if (overwrite) {
            // We never found a part with a finite bounding box
            box = AABox(Vector3::zero());
        }
    }
}

ArticulatedModel::Part::~Part() {
}

} // namespace G3D
