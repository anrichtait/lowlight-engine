/**
  \file tools/viewer/ArticulatedViewer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2024, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "ArticulatedViewer.h"
#include "App.h"

shared_ptr<GFont> ArticulatedViewer::m_font;

// Useful for debugging material assignments
static const bool mergeMaterials = true;

ArticulatedViewer::ArticulatedViewer() :
    m_numFaces(0),
    m_numVertices(0),
    m_selectedPart(nullptr),
    m_selectedMesh(nullptr),
    m_selectedTriangleIndex(0),
    m_time(0.0f) {

    if (isNull(m_font)) {
        m_font          = GFont::fromFile(System::findDataFile("arial.fnt"));        
    }
}

ArticulatedViewer::~ArticulatedViewer() {
    ArticulatedModel::clearCache();
    Texture::clearCache();
    UniversalMaterial::clearCache();
}

static const float VIEW_SIZE = 30.0f;

void ArticulatedViewer::onInit(const String& filename, App* app) {
    ArticulatedModel::clearCache();
    Texture::clearCache();

    m_model = nullptr;
    m_filename = filename;

    m_selectedPart   = nullptr;
    m_selectedMesh   = nullptr;
    m_selectedTriangleIndex = -1;
    m_numFaces       = 0;
    m_numVertices    = 0;
    m_shadowMapDirty = true;

    UniversalMaterial::clearCache();
    
    const RealTime start = System::time();
    if (toLower(FilePath::ext(filename)) == "any") {

        if (toLower(FilePath::ext(FilePath::base(filename))) == "universalmaterial") {

            // Assume that this is an .UniversalMaterial.Any file. Load a square and apply the material
            Any any(Any::TABLE, "ArticulatedModel::Specification");
            any["filename"] = "model/mori_knob/mori_knob.zip/testObj.obj";

            Any preprocess(Any::ARRAY);

            preprocess.append(PARSE_ANY(removeMesh("LTELogo/LTELogo");));
            preprocess.append(PARSE_ANY(removeMesh("Material/Material");));
            preprocess.append(PARSE_ANY(scaleAndOffsetTexCoord0("BackGroundMat/BackGroundMat", 2.5, Vector2(0,0));));
            preprocess.append(PARSE_ANY(scaleAndOffsetTexCoord0("OuterMat/OuterMat", 3, Vector2(0,0));));
            preprocess.append(PARSE_ANY(scaleAndOffsetTexCoord0("InnerMat/InnerMat", 1.6, Vector2(0,0));));
            Any setMaterial(Any::ARRAY, "setMaterial");
            setMaterial.append(Any(Any::ARRAY, "all"));
            setMaterial.append(Any::fromFile(filename));
            preprocess.append(setMaterial);
            preprocess.append(PARSE_ANY(mergeAll(ALL, NONE);));
            preprocess.append(PARSE_ANY(setTwoSided(all(), true);));
            preprocess.append(PARSE_ANY(moveBaseToOrigin();));
            preprocess.append(PARSE_ANY(setCFrame(root(), CFrame::fromXYZYPRDegrees(0, 0, 0, 90, 0, 0));));

            any["preprocess"] = preprocess;

            m_model = ArticulatedModel::create(ArticulatedModel::Specification(any));

            const CFrame& F = CFrame::fromXYZYPRDegrees(-8.2499f, -3.8855f, -1.0488f, -110.44f, -17.228f, 0);
            app->debugCamera()->setFrame(F); 
            app->cameraManipulator()->setFrame(F); 
        } else {

            // Assume that this is an .ArticulatedModel.Any file
            Any any;
            any.load(filename);

            m_model = ArticulatedModel::create(ArticulatedModel::Specification(any));
        }
    } else {
        Any any(Any::TABLE, "ArticulatedModel::Specification");
        any["filename"] = filename;

        // Prevent merging for material debugging
        if (! mergeMaterials) {
            any["meshMergeOpaqueClusterRadius"] = 0;
        }

        const shared_ptr<DefaultRenderer>& renderer = dynamic_pointer_cast<DefaultRenderer>(app->renderer());
        if (notNull(renderer) && mergeMaterials && renderer->orderIndependentTransparency()) {
            any["meshMergeTransmissiveClusterRadius"] = finf();
        }

        // any["stripMaterials"] = true;
        m_model = ArticulatedModel::create(any);
    }
    debugPrintf("%s loaded in %f seconds\n", filename.c_str(), System::time() - start);


    Array<shared_ptr<Surface> > arrayModel;
    if (m_model->usesSkeletalAnimation()) {
        Array<String> animationNames;
        m_model->getAnimationNames(animationNames);
        // TODO: Add support for selecting animations.
        m_model->getAnimation(animationNames[0], m_animation); 
        m_animation.getCurrentPose(0.0f, m_pose);
    } 
    
    m_model->pose(arrayModel, CFrame(), CFrame(), nullptr, &m_pose, &m_pose, Surface::ExpressiveLightScatteringProperties());

    m_model->countTrianglesAndVertices(m_numFaces, m_numVertices);
    
    m_scale = 1;
    m_offset = Vector3::zero();
    bool overwrite = true;
    
    // Find the size of the bounding box of the entire model
    AABox bounds;
    if (arrayModel.size() > 0) {
        
        for (int x = 0; x < arrayModel.size(); ++x) {		
            
            //merges the bounding boxes of all the seperate parts into the bounding box of the entire object
            AABox temp;
            CFrame cframe;
            arrayModel[x]->getCoordinateFrame(cframe);
            arrayModel[x]->getObjectSpaceBoundingBox(temp);
            Box partBounds = cframe.toWorldSpace(temp);
            
            // Some models have screwed up bounding boxes
            if (partBounds.extent().isFinite()) {
                if (overwrite) {
                    partBounds.getBounds(bounds);
                    overwrite = false;
                } else {
                    partBounds.getBounds(temp);
                    bounds.merge(temp);
                }
            }
        }
        
        if (overwrite) {
            // We never found a part with a finite bounding box
            bounds = AABox(Vector3::zero());
        }
        
        Vector3 extent = bounds.extent();
        Vector3 center = bounds.center();
        
        // Scale to X units
        float scale = 1.0f / max(extent.x, max(extent.y, extent.z));
        
        if (scale <= 0) {
            scale = 1;
        }

        if (! isFinite(scale)) {
            scale = 1;
        }

        m_scale = scale;
        scale *= VIEW_SIZE;
        m_offset = -scale * center;
        

        if (! center.isFinite()) {
            center = Vector3();
        }

        // Transform parts in-place
        m_model->scaleWholeModel(scale);

        ArticulatedModel::CleanGeometrySettings csg;
        // Merging vertices is slow and topology hasn't changed at all, so preclude vertex merging
        csg.allowVertexMerging = false; 
        m_model->cleanGeometry(csg);
    }

    // Get the newly transformed animation
    if (m_model->usesSkeletalAnimation()) {
        Array<String> animationNames;
        m_model->getAnimationNames(animationNames);
        // TODO: Add support for selecting animations.
        m_model->getAnimation(animationNames[0], m_animation); 
        m_animation.getCurrentPose(0.0f, m_pose);
    } 

    //saveGeometry();
}


static void writeXYZ(TextOutput& file, const Vector3& v) {
    file.printf("xyz(%f, %f, %f)", v.x, v.y, v.z);
}


void ArticulatedViewer::saveGeometry() {
    const bool pyxlscript = true;

    Array<shared_ptr<Surface>> surfaceArray;
    m_model->pose(surfaceArray, Point3::zero(), Point3::zero(), nullptr, &m_pose, &m_pose, Surface::ExpressiveLightScatteringProperties());
   
    // Extract indexed triangle array
    CPUVertexArray packedVertexArray;
    Array<Tri> triArray;
    Surface::getTris(surfaceArray, packedVertexArray, triArray);

    Array<int> indexArray;
    for (const Tri& tri : triArray) {
        for (int i = 0; i < 3; ++i) {
            indexArray.push(tri.getIndex(i));
        }
    }

    Array<Point3> positionArray;

    // Weld into a single set of faces, ignoring normals and texcoords, for raw geo
    {
        Array<Point3> rawPositionArray;
        for (const CPUVertexArray::Vertex& vertex : packedVertexArray.vertex) {
            rawPositionArray.push(vertex.position);
        }
        
        // Weld
        Array<int> toNew;
        Array<int> toOld;
        MeshAlg::computeWeld(rawPositionArray, positionArray, toNew, toOld);
        
        // Map index array to post-weld scheme
        for (int i = 0; i < indexArray.size(); ++i) {
            indexArray[i] = toNew[indexArray[i]];
        }
    }
    
    // Compute adjacency
    Array<MeshAlg::Face> faceArray;
    Array<MeshAlg::Edge> edgeArray;
    Array<MeshAlg::Vertex> vertexArray;
    MeshAlg::computeAdjacency(positionArray, indexArray, faceArray, edgeArray, vertexArray);

    Array<Vector3> faceNormalArray;
    MeshAlg::computeFaceNormals(positionArray, faceArray, faceNormalArray);

    if (pyxlscript) {
        const String& filename = "out.pyxl";
        debugPrintf("Saving %s\n", FilePath::concat(FileSystem::currentDirectory(), filename).c_str());
        TextOutput file(filename);
        file.writeSymbol("{");
        file.writeNewline();
        file.pushIndent();
        {
            file.writeSymbols("vertex_array", ":", "[");
            file.writeNewline();
            file.pushIndent();
            for (const Point3& position : positionArray) {
                writeXYZ(file, position);
                file.printf(",");
                file.writeNewline();
            }
            file.popIndent();
            file.writeSymbols("],");
            file.writeNewline();

            file.writeSymbols("face_array", ":", "[");
            file.writeNewline();
            file.pushIndent();
            for (int f = 0; f < faceArray.size(); ++f) {
                const MeshAlg::Face& face = faceArray[f];
                file.writeSymbol("{");
                file.writeNewline();
                file.pushIndent();
                file.writeSymbols("index_array", ":");
                file.printf("[%d, %d, %d],", face.vertexIndex[0], face.vertexIndex[1], face.vertexIndex[2]);
                file.writeNewline();

                file.writeSymbols("normal", ":");
                writeXYZ(file, faceNormalArray[f]);
                file.writeNewline();
                file.popIndent();
                file.writeSymbols("},");
                file.writeNewline();
            }
            file.popIndent();
            file.writeSymbols("],");
            file.writeNewline();

            file.writeSymbols("edge_array", ":", "[");
            file.writeNewline();
            file.pushIndent();
            for (const MeshAlg::Edge& edge : edgeArray) {
                file.writeSymbols("{");
                
                file.writeNewline();
                file.pushIndent();
                file.writeSymbols("index_array", ":");
                file.printf("[%d, %d],", edge.vertexIndex[0], edge.vertexIndex[1]);
                file.writeNewline();
                file.writeSymbols("face_index_array", ":");
                file.printf("[%d, %d],", edge.faceIndex[0], edge.faceIndex[1]);
                file.writeNewline();
                file.writeSymbols("normal", ":");

                const Vector3& n0 = (edge.faceIndex[0] != MeshAlg::Face::NONE) ? faceNormalArray[edge.faceIndex[0]] : Vector3::zero();
                const Vector3& n1 = (edge.faceIndex[1] != MeshAlg::Face::NONE) ? faceNormalArray[edge.faceIndex[1]] : Vector3::zero();
                writeXYZ(file, (n0 + n1).directionOrZero());

                file.writeNewline();
                file.popIndent();
                file.writeSymbols("},");
                file.writeNewline();
            }
            file.popIndent();
            file.writeSymbols("]");
            file.writeNewline();
        }
        file.popIndent();
        file.writeSymbol("}");
        file.writeNewline();
        file.commit();
    }
}


static void printHierarchy
(const shared_ptr<ArticulatedModel>& model,
 ArticulatedModel::Part*             part,
 const String&                       indent) {
    
    screenPrintf("%s\"%s\")\n", indent.c_str(), part->name.c_str());
    for (int i = 0; i < model->meshArray().size(); ++i) {
        if (model->meshArray()[i]->logicalPart == part) {
            screenPrintf("%s  Mesh \"%s\"\n", indent.c_str(), model->meshArray()[i]->name.c_str());
        }
    }

    for (int i = 0; i < part->childArray().size(); ++i) {
        printHierarchy(model, part->childArray()[i], indent + "  ");
    }
}


void ArticulatedViewer::onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) {
    Viewer3D::onPose(posed3D, posed2D);

    m_model->pose(posed3D, m_offset, m_offset, nullptr, &m_pose, &m_pose, Surface::ExpressiveLightScatteringProperties());
}
    

void ArticulatedViewer::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surfaceArray) {
    Viewer3D::onGraphics3D(rd, surfaceArray);

    Array<Point3> skeletonLines;    
    m_model->getSkeletonLines(m_pose, m_offset, skeletonLines);
    
    if (skeletonLines.size() > 0) {
        rd->pushState(); {
            rd->setObjectToWorldMatrix(CFrame());
            rd->setDepthTest(RenderDevice::DEPTH_ALWAYS_PASS);
            for (int i = 0; i < skeletonLines.size(); i += 2) {
                Draw::lineSegment(LineSegment::fromTwoPoints(skeletonLines[i], skeletonLines[i + 1]), rd, Color3::red());
            }
        } rd->popState();
    }
    

    //Surface::renderWireframe(rd, posed3D);

    if (notNull(m_selectedMesh)) {
        // Find the index array that matches the selected mesh and render it
        for (int p = 0; p < surfaceArray.size(); ++p) {
            const shared_ptr<UniversalSurface>& s = dynamic_pointer_cast<UniversalSurface>(surfaceArray[p]);

            if (s && s->gpuGeom()->index == m_selectedMesh->gpuIndexArray) {
                // These have the same index array, so they must be the same surface
                s->renderWireframeHomogeneous(rd, Array<shared_ptr<Surface> >(s), Color3::green(), false);
                break;
            }
        }
    }

    if (! mergeMaterials) {
        screenPrintf("Mesh merging by material DISABLED in this build. Models will render slowly but maintain assignments.\n");
    }

    screenPrintf("[Shown scaled by %f and offset by (%f, %f, %f)]\n",
                 m_scale, m_offset.x, m_offset.y, m_offset.z);
    
    screenPrintf("Model Faces: %d,  Vertices: %d\n", m_numFaces, m_numVertices);
    if (notNull(m_selectedPart)) {
        screenPrintf(" Selected Part `%s', Mesh `%s' (Ctrl-C to copy), Material `%s', cpuIndexArray[%d...%d]\n", 
                     m_selectedPart->name.c_str(), 
                     m_selectedMesh->name.c_str(), 
					 m_selectedMesh->material->name().c_str(),
                     m_selectedTriangleIndex, m_selectedTriangleIndex + 2);
        screenPrintf(" Selected part->cframe = %s\n",
                     m_selectedPart->cframe.toXYZYPRDegreesString().c_str());
    }

    screenPrintf("Hierarchy:");
    // Hierarchy (could do this with a PartCallback)
    for (int i = 0; i < m_model->rootArray().size(); ++i) {
        printHierarchy(m_model, m_model->rootArray()[i], "");
    }
}


void ArticulatedViewer::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
   m_time += sdt;
   if (m_model->usesSkeletalAnimation()) {
        m_animation.getCurrentPose(m_time, m_pose);
   }
}


bool ArticulatedViewer::onEvent(const GEvent& e, App* app) {
    if (Viewer3D::onEvent(e, app)) {
        // For refreshing the file
        return true;
    }

    if ((e.type == GEventType::MOUSE_BUTTON_DOWN) && (e.button.button == 0) && ! app->userInput->keyDown(GKey::LCTRL)) {
        // Intersect all tri lists with the ray from the camera
        const Ray& ray = app->activeCamera()->worldRay(e.button.x, e.button.y, 
            app->renderDevice->viewport());

        m_selectedPart = nullptr;
        m_selectedMesh = nullptr;
        m_selectedTriangleIndex = -1;
        Model::HitInfo hitInfo;
        float distance = finf();
        const bool hit = m_model->intersect(ray, m_offset, distance, hitInfo, nullptr, nullptr);

        if (hit) {
            m_selectedMesh = m_model->mesh(hitInfo.meshID);
            m_selectedTriangleIndex = hitInfo.primitiveIndex;
            // Output the name of the mesh so that multiple selections can easily
            // be copied from the debug window for processing by other tools
            debugPrintf("\"%s\",\n", m_selectedMesh->name.c_str());
        }

        if (notNull(m_selectedMesh)) {
            m_selectedPart = m_selectedMesh->logicalPart;
        }
        return hit;
    } else if  ((e.type == GEventType::KEY_DOWN) && (e.key.keysym.sym == 'c') && (app->userInput->keyDown(GKey::LCTRL) || app->userInput->keyDown(GKey::RCTRL))) {
        OSWindow::setClipboardText(m_selectedMesh->name);
        return true;
    }

    return false;
}
