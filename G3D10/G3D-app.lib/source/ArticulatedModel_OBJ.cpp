/**
  \file G3D-app.lib/source/ArticulatedModel_OBJ.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/ArticulatedModel.h"
#include "G3D-base/ParseOBJ.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/Stopwatch.h"

namespace G3D {

static void stripMaterials(ParseOBJ& parseData) {
    const shared_ptr<ParseMTL::Material>& defaultMaterial = ParseMTL::Material::create();

    // Collapse the groups
    for (ParseOBJ::GroupTable::Iterator git = parseData.groupTable.begin();
        git.isValid();
        ++git) {
        const shared_ptr<ParseOBJ::Group>& group = git->value;

        // For each mesh
        for (ParseOBJ::MeshTable::Iterator mit = group->meshTable.begin();
            mit.isValid();
            ++mit) {

            shared_ptr<ParseOBJ::Mesh>& mesh = mit->value;
            mesh->material = defaultMaterial;
        }
    }
}


/** Rewrites the group table so that there is a single group per material, 
   and that group contains a single mesh. This is OBJ-specific for performance
   and takes advantage of OBJ files containing the material data per group and
   thus avoids expanding the data during parsing.

   G3D::ArticulatedModel also has a general-purpose mesh merger, but it does
   not run if this merger runs.*/
static void mergeGroupsAndMeshesByMaterial(ParseOBJ& parseData) {
    // Construct one mesh per material into this table,
    // and then rebuild the groups at the end
    Table<String, shared_ptr<ParseOBJ::Mesh>> newMeshTable;

    // Maps the name of each material to the first alphabetically-sorted group
    // name that uses that material for consistent mesh naming after merges.
    Table<String, String> materialNameToLowestGroupName;

    // Collapse the groups
    for (ParseOBJ::GroupTable::Iterator groupIterator = parseData.groupTable.begin();
        groupIterator.isValid();
        ++groupIterator) {
        const shared_ptr<ParseOBJ::Group>& group = groupIterator->value;

        // For each mesh
        for (ParseOBJ::MeshTable::Iterator meshIterator = group->meshTable.begin();
            meshIterator.isValid();
            ++meshIterator) {

            const shared_ptr<ParseOBJ::Mesh>& srcMesh = meshIterator->value;
            const String& materialName = srcMesh->material->name;

            // Choose the alphabetically lowest name for the group, to be consistent with 
            // the general-purpose merger
            String& lowestGroupName = materialNameToLowestGroupName.getCreate(materialName);
            if (lowestGroupName.empty() || (group->name < lowestGroupName)) {
                lowestGroupName = group->name;
            }

            shared_ptr<ParseOBJ::Mesh>& dstMesh = newMeshTable.getCreate(materialName);
            if (isNull(dstMesh)) {
                // First time
                dstMesh = ParseOBJ::Mesh::create();
                dstMesh->material = srcMesh->material;  
            }

            dstMesh->faceArray.append(srcMesh->faceArray);
        }
    }

    // Rebuild the group table from the meshes
    parseData.groupTable.clear();
    for (Table<String, shared_ptr<ParseOBJ::Mesh>>::Iterator meshIterator = newMeshTable.begin();
        meshIterator.isValid();
        ++meshIterator) {

        const shared_ptr<ParseOBJ::Mesh>&  mesh  = meshIterator->value;
        const shared_ptr<ParseOBJ::Group>& group = ParseOBJ::Group::create();

        // Use the new canonical name for this group
        group->name = materialNameToLowestGroupName[mesh->material->name];
        if (parseData.groupTable.containsKey(group->name)) {
            // Make the name unique
            group->name += "/" + mesh->material->name;
        }

        group->meshTable.set(mesh->material, mesh);
        debugAssertM(! parseData.groupTable.containsKey(group->name),
            String("Merged group '") + group->name + "' was already created");
        parseData.groupTable.set(group->name, group);
    }
    debugAssert(newMeshTable.size() == parseData.groupTable.size());
}

/** \param basePath Resolve relative paths to here
\cite http://www.fileformat.info/format/material/

MTL illum constants:
0     Color on and Ambient off 

1     Color on and Ambient on 

2     Highlight on 

3     Reflection on and Ray trace on 

4     Transparency: Glass on 
     Reflection: Ray trace on 

5     Reflection: Fresnel on and Ray trace on 

6     Transparency: Refraction on 
     Reflection: Fresnel off and Ray trace on 

7     Transparency: Refraction on 
     Reflection: Fresnel on and Ray trace on 

8     Reflection on and Ray trace off 

9     Transparency: Glass on 
     Reflection: Ray trace off 

10     Casts shadows onto invisible surfaces 
*/
UniversalMaterial::Specification ArticulatedModel::Specification::convertMTLToUniversalMaterialSpecification
(   const shared_ptr<ParseMTL::Material>& m,
    AlphaFilter a,
    RefractionHint r) const {

    const ArticulatedModel::Specification& modelSpec = *this;

    UniversalMaterial::Specification s;
    String filename;
    String alphaFilename;

    s.setAlphaHint(a);
    s.setRefractionHint(r);

    if (! modelSpec.objOptions.forceMaterialsWhite) {
        // Map OBJ materials to G3D UniversalMaterial
        filename      = ArticulatedModel::resolveRelativeFilename(m->Kd.map, m->basePath);
        alphaFilename = ArticulatedModel::resolveRelativeFilename(m->map_d,  m->basePath);
    }

    // TODO: preserve alpha under forceMaterialsWhite
    if (! modelSpec.objOptions.forceMaterialsWhite) {
        if ((! filename.empty()) && FileSystem::exists(filename)) {
            Texture::Specification t(filename, true);
            if ((! alphaFilename.empty()) && FileSystem::exists(alphaFilename)) {
                t.encoding.readMultiplyFirst = Color4(Color3(m->Kd.mm.y), m->d);
                t.alphaFilename = alphaFilename;
                s.setLambertian(t);
            } else {
                // OBJ texture map overrides the constant color in the spec
                t.encoding.readMultiplyFirst.a = m->d;
                s.setLambertian(t);
            } 
        } else if ((! alphaFilename.empty()) && FileSystem::exists(alphaFilename)) {
            Texture::Specification t("<white>", true);
            t.alphaFilename = alphaFilename;
            t.encoding.readMultiplyFirst = Color4(m->Kd.constant, m->d);
            s.setLambertian(t);
        } else {
            s.setLambertian(Texture::Specification(Color4(m->Kd.constant, m->d)));
        }
    }

    if (! modelSpec.objOptions.forceMaterialsWhite) {
        filename = ArticulatedModel::resolveRelativeFilename(m->Ks.map, m->basePath);
        {
            if (filename.empty()) {
                filename = "<white>";
            }

            Texture::Specification t(filename, true);

            // Detect negatives and handle them sanely
            if (m->Ks.constant.r < 0) {
                m->Ks.constant = Color3(0.5f);
            }
            alwaysAssertM(m->Ks.constant.r >= 0, "Negative Ks persisted from ParseMTL");

            // Keep glossiness low until the specified value is really high, and then give up and allow it to be high.
            // This compensates for the fact that most MTL files were authored for LDR rendering and are too glossy to compensate.
            t.encoding.readMultiplyFirst = Color4((m->Ks.constant.pow(6.0f) * 0.7f + m->Ks.constant * 0.3f) * m->Ks.mm.y, 1.0f);

            if (! m->Ks.map.empty() && ! FileSystem::exists(filename)) {
                // Make black if missing
                debugPrintf("Warning: cannot find texture '%s'\n", filename.c_str());
                t.filename = "<white>";
                t.encoding.readMultiplyFirst = Color4::zero();
            }

            if (m->Ns >= 99.0f) {
                // Force large values to mirror
                t.encoding.readMultiplyFirst.a = 1.0f;
            } else {
                t.encoding.readMultiplyFirst.a = pow(m->Ns / 100.0f, 1.0f);
            }
            s.setGlossy(t);
        }

        if (m->illum == 4 || m->illum == 6 || m->illum == 7 || m->illum == 9) {
            s.setTransmissive(Texture::Specification(Color3::white() - m->Tf));

            // Index of refraction (assume air)
            if (! modelSpec.objOptions.stripRefraction) {
                s.setEta(m->Ni, 1.0f);
            }
        }
    }

    // Map OBJ model to G3D shading 
    filename = ArticulatedModel::resolveRelativeFilename(m->lightMap, m->basePath);
    if (! filename.empty() && FileSystem::exists(filename)) {
        Texture::Specification textureSpec(filename, true);
        // This leverages the texture cache to avoid lightMap duplication, as opposed to using Texture::fromFile
        s.setLightMaps(Texture::create(textureSpec));
    }

    BumpMap::Settings bumpSettings;
    bumpSettings.bias = m->bump.mm.x;
    bumpSettings.scale = m->bump.mm.y * 0.008f;
    bumpSettings.iterations = modelSpec.objOptions.materialOptions.defaultBumpMapIterations;
    s.setBump(ArticulatedModel::resolveRelativeFilename(m->bump.map, m->basePath), bumpSettings, 4.0f, modelSpec.invertPrecomputedNormalYAxis);
    
    filename = ArticulatedModel::resolveRelativeFilename(m->Ke.map, m->basePath);
    if (filename.empty() || ! FileSystem::exists(filename)) {
        s.setEmissive(Texture::Specification(m->Ke.constant));
    } else {
        if (m->Ke.constant == Color3(0.0f)) {
            m->Ke.constant = Color3(1.0f);
        }
        Texture::Specification textureSpec(filename, true);
        textureSpec.encoding.readMultiplyFirst = Color4(m->Ke.constant * m->Ke.mm.y, 1.0f);
        s.setEmissive(textureSpec);
    }

    if (! m->interpolateMode.empty()) {
        Sampler sampler;
        sampler.interpolateMode = InterpolateMode(m->interpolateMode);
        s.setSampler(sampler);
    }

    // s.setSampler(modelSpec.objOptions.sampler);

    return s;
}


/** Flip texture coordinates from the OBJ to the G3D convention */
inline static Point2 OBJToG3DTex(const Vector2& t) {
    return Vector2(t.x, 1.0f - t.y);
}


inline static Point2 G3DToOBJTex(const Vector2& t) {
    return Vector2(t.x, (-1.0f * t.y) + 1.0f);
}


const bool timeArticulatedModelLoad = false;

void ArticulatedModel::loadOBJ(const Specification& specification) {
    // During loading, we make no attempt to optimize the mesh's topology.  We leave that until the
    // Parts have been created.  The vertex arrays are therefore much larger than they
    // need to be.
    ContinuousStopwatch timer;
    timer.setEnabled(timeArticulatedModelLoad);

    ParseOBJ parseData;
    {
        BinaryInput bi(specification.filename, G3D_LITTLE_ENDIAN);
        timer.printElapsedTime(" open file");
        parseData.parse(bi, specification.objOptions);

        m_mtlArray = parseData.mtlArray;
        //adds a dummy entry to the end of the array so that models loaded from an OBJ without textures can be distinguished from other models
        //temporary fix until saveOBJ can save from other types of models
        m_mtlArray.append("");

        timer.printElapsedTime(" parse OBJ");
        // Let the BinaryInput go out of scope, reclaiming its memory
    }
    bool hasTexCoord1s = parseData.texCoord1Array.size() > 0;
    alwaysAssertM(!hasTexCoord1s || parseData.texCoord1Array.size() == parseData.texCoord0Array.size(), 
                    "Mismatching #s of texCoord0 and texCoord1");

    Part* part      = addPart("root");
    Geometry* geom  = addGeometry("root");

    if (specification.stripMaterials) {
        stripMaterials(parseData);
    }

    if ((specification.meshMergeOpaqueClusterRadius == finf()) && 
        (specification.meshMergeTransmissiveClusterRadius == finf())) {
        // Go ahead and reduce the number of groups at parse
        // time to reduce downstream processing
        mergeGroupsAndMeshesByMaterial(parseData);
        timer.printElapsedTime(" merge materials");
    }

    int numSpecifiedNormals = 0;
    int numSpecifiedTexCoord0s = 0;

    // Preallocate enough memory to store all the faces if the faces are triangles
    int numVertices = 0;

    // Iterate serially over all groups and meshes, flattening them from the table
    // iterators into a flat array that we can then process in parallel.
    // All groups together form a single ArticulatedModel::Geometry.  Each mesh in each group
    // forms a single ArticulatedModel::Mesh.

    // Parallel arrays, where
    // uniqueMaterialSpecificationArray[materialSpecificationIndexArray[m]] == the material for dstMeshArray[m]
    Array<shared_ptr<ParseOBJ::Mesh>> srcMeshArray;
    Array<Mesh*> dstMeshArray;
    Array<int> materialSpecificationIndexArray;

    // Not parallel to the other three arrays. This contains only unique material specifications.
    Table<UniversalMaterial::Specification, int> materialSpecificationIndexTable;
    Array<UniversalMaterial::Specification> uniqueMaterialSpecificationArray;

    // Sort the groups to ensure ordering is deterministic across runs.
    Array<String> groupNames;
    parseData.groupTable.getKeys(groupNames);
    groupNames.sort();

    for (const String& name : groupNames) {

        const shared_ptr<ParseOBJ::Group>& group = parseData.groupTable[name];

        // Sort the (obj) meshes to ensure ordering is deterministic across runs.
        Array<shared_ptr<ParseMTL::Material>> meshMaterials;
        group->meshTable.getKeys(meshMaterials);
        meshMaterials.sort([&](const shared_ptr<ParseMTL::Material>& a, const shared_ptr<ParseMTL::Material>& b) {
            return a->name > b->name;
        });

        for(const shared_ptr<ParseMTL::Material> material : meshMaterials) {
            const shared_ptr<ParseOBJ::Mesh>& srcMesh = group->meshTable[material];

            numVertices += srcMesh->faceArray.size() * 3;
            srcMeshArray.append(srcMesh);

            // Construct the ArticulatedModel::Mesh for this group + mesh combination
            dstMeshArray.append(addMesh(group->name + "/" + srcMesh->material->name, part, geom));

            if (specification.stripMaterials) {
                materialSpecificationIndexArray.append(0);
            } else {
                const UniversalMaterial::Specification& s = specification.convertMTLToUniversalMaterialSpecification(srcMesh->material, specification.alphaFilter, specification.refractionHint);
                bool created = false;
                int& materialIndex = materialSpecificationIndexTable.getCreate(s, created);
                if (created) {
                    materialIndex = uniqueMaterialSpecificationArray.size();
                    uniqueMaterialSpecificationArray.append(s);
                }
                materialSpecificationIndexArray.append(materialIndex);
            }

        } // for each mesh in the group

    } // for each group
    geom->cpuVertexArray.vertex.reserve(numVertices);
    if (hasTexCoord1s) {
        geom->cpuVertexArray.texCoord1.reserve(numVertices);
    }
    
    // Discard the table itself now that we have the meshes
    parseData.groupTable.clear();

    timer.printElapsedTime(" allocate meshes");

    // Create the unique materials
    Array<shared_ptr<UniversalMaterial>> uniqueMaterialArray;
    if (! specification.stripMaterials) {
        uniqueMaterialArray.resize(uniqueMaterialSpecificationArray.size());
        for (int m = 0; m < uniqueMaterialArray.size(); ++m) {
            uniqueMaterialArray[m] = UniversalMaterial::create(uniqueMaterialSpecificationArray[m]);
        }
    } else {
        uniqueMaterialArray.append(UniversalMaterial::create());
    }
    timer.printElapsedTime(" load materials");

    // For each mesh
    for (int m = 0; m < srcMeshArray.size(); ++m) {
        const shared_ptr<ParseOBJ::Mesh>& srcMesh = srcMeshArray[m];

        Mesh* mesh = dstMeshArray[m];
        mesh->material = uniqueMaterialArray[materialSpecificationIndexArray[m]];

        // For each face
        Array<ParseOBJ::Face>& faceArray = srcMesh->faceArray;
        mesh->cpuIndexArray.reserve(faceArray.size()); // Allocate space for indices up front.
        for (int f = 0; f < faceArray.size(); ++f) {
            const ParseOBJ::Face& face = faceArray[f];

            // Index of the first vertex that we'll add for this face
            const int prevNumVertices = geom->cpuVertexArray.size();

            // For each vertex
            for (int v = 0; v < face.size(); ++v) {
                const ParseOBJ::Index& index = face[v];
                debugAssert(index.vertex != ParseOBJ::UNDEFINED);

                CPUVertexArray::Vertex& vertex = geom->cpuVertexArray.vertex.next();

                vertex.position = parseData.vertexArray[index.vertex];

                if (index.normal != ParseOBJ::UNDEFINED) {
                    vertex.normal = parseData.normalArray[index.normal];
                    ++numSpecifiedNormals;
                } else {
                    vertex.normal = Vector3::nan();
                }

                if (index.texCoord != ParseOBJ::UNDEFINED) {
                    vertex.texCoord0 = OBJToG3DTex(parseData.texCoord0Array[index.texCoord]);
                    ++numSpecifiedTexCoord0s;
                    if (hasTexCoord1s) {
                        geom->cpuVertexArray.texCoord1.append(Point2unorm16(OBJToG3DTex(parseData.texCoord1Array[index.texCoord])));
                    }
                } else {
                    vertex.texCoord0 = Point2::zero();
                }

                // We have no tangent, so force it to NaN
                vertex.tangent = Vector4::nan();
            } // for each vertex

            // Tessellate the polygon into triangles, writing to both the part index array
            // and the mesh index array.
            for (int t = 2; t < face.size(); ++t) {
                int i = prevNumVertices + t - 2;
                mesh->cpuIndexArray.append(prevNumVertices, i + 1, i + 2);
            } // for each triangle in the face
        } // for each face

        // Remove old face data from memory to free space
        faceArray.clear(true);
    } // for each mesh
    timer.printElapsedTime(" copy vertex data");


    // If there are any texture coordinates, consider them all valid.  Only some of the
    // meshes may have texture coordinates, but those may need tangents and texcoords.
    // TODO: part->m_hasTexCoord0 = (numSpecifiedTexCoord0s > 0);
    geom->cpuVertexArray.hasTexCoord1 = hasTexCoord1s && (numSpecifiedTexCoord0s > 0);

    // Make any mesh that has partial coverage or transmission two-sided (OBJ-specific logic)
    for (int m = 0; m < m_meshArray.size(); ++m) {
        Mesh* mesh = m_meshArray[m];
        const shared_ptr<UniversalMaterial>& material = mesh->material;
        const shared_ptr<UniversalBSDF>& bsdf = material->bsdf();

        if (bsdf->conservativelyHasTransparency()) {
            mesh->twoSided = true;
        }
    }
    timer.printElapsedTime(" detect two-sided meshes");

    // Debugging code
    if (false) {
        // Code for printing the imported vertices
        debugPrintf("** Vertices:\n");
        for (int i = 0; i < geom->cpuVertexArray.vertex.size(); ++i) {
            const CPUVertexArray::Vertex& vertex = geom->cpuVertexArray.vertex[i];
            debugPrintf(" %d: %s %s %s %s\n", i, 
                vertex.position.toString().c_str(), vertex.normal.toString().c_str(),
                vertex.tangent.toString().c_str(), vertex.texCoord0.toString().c_str());
        }
        debugPrintf("\n");

        // Code for dumping the imported indices
        debugPrintf("** Indices:\n");
        for (int m = 0; m < m_meshArray.size(); ++m) {
            const Mesh* mesh = m_meshArray[m];
            debugPrintf(" Mesh %s\n", mesh->name.c_str());
            for (int i = 0; i < mesh->cpuIndexArray.size(); i += 3) {
                debugPrintf(" %d-%d: %d %d %d\n", i, i + 2, mesh->cpuIndexArray[i], mesh->cpuIndexArray[i + 1], mesh->cpuIndexArray[i + 2]);
            }
            debugPrintf("\n");
        }
        debugPrintf("\n");
    }

    timer.printElapsedTime(" OBJ-specific processing");
}

//compresses the vertex array verts, returns a table of the new index mapping from the uncompressed to the compressed array, and updates the offset to be used by any further geometries
template<class T> static void compressVertices(Table<int,int>& indexMap, Array<T>& verts, int offset) {
    Table<T, int> vertMap;
    Array<T> newArray;
    for (int v = 0; v < verts.size(); ++v) {
        bool created;
        int& i = vertMap.getCreate(verts[v], created);
        if (created) {
            i = newArray.size();
            newArray.append(verts[v]);
        }
        indexMap.set(v, i + offset);
    }
    verts = newArray;
    offset += verts.size();
}


void ArticulatedModel::saveOBJ(const String& filename) {
    const bool hasTexCoord0 = false;

    TextOutput::Settings opt;
    opt.wordWrap = TextOutput::Settings::WRAP_NONE;
    TextOutput to(filename, opt);

    // OBJ header
    to.printf("%s %s %s", "# Model Generated by G3D \n\n#\n#", FilePath::baseExt(filename).c_str(),"\n#\n\n");
    if (m_mtlArray.length() == 0) {
        debugPrintf("Warning: saveOBJ called on a model not loaded from an OBJ file. Materials will not be saved.\n");
    }

    // Declare the MTL files used (usually there is at most one)
    for (int i = 0; i < m_mtlArray.length() - 1; ++i) {
        to.printf("%s %s\n", "mtllib", m_mtlArray[i].c_str());
    }
    to.writeNewline();

    // Tables which map indices in mesh index arrays to real vertices
    Array<Table<int, int> > positionMaps;
    Array<Table<int, int> > normalMaps;
    Array<Table<int, int> > texCoordMaps;
   
    //keep track of offset of a meshes' index arrays to the overall index (will only be 1 unless there are multiple geometries)
    int basePosIndex  = 1;
    int baseNormIndex = 1; 
    int baseTexIndex  = 1; 
    // Print vertex arrays
    for (int g = 0; g < m_geometryArray.size(); ++g) {
        Geometry* geom = m_geometryArray[g];

        //compress positions
        Array<Vector3> positions;
        for (int v = 0; v < geom->cpuVertexArray.size(); ++v) {
            positions.append(geom->cpuVertexArray.vertex[v].position);
        }
        Table<int, int> newPosMap;
        compressVertices<Vector3>(newPosMap, positions, basePosIndex);
        positionMaps.append(newPosMap);

        //normals
        Array<Vector3> normals;
        for (int v = 0; v < geom->cpuVertexArray.size(); ++v) {
            normals.append(geom->cpuVertexArray.vertex[v].normal);
        }
        Table<int, int> newNormMap;
        compressVertices<Vector3>(newNormMap, normals, baseNormIndex);
        normalMaps.append(newNormMap);

        Array<Vector2> texCoord0s;

        // All TexCoords packed together
        Array<Vector4> texCoord01s;
        Table<int, int> newTexMap;
        bool hasTexCoord1 = geom->cpuVertexArray.hasTexCoord1;
        if (hasTexCoord1) {
            debugAssert(hasTexCoord0);
            for (int v = 0; v < geom->cpuVertexArray.size(); ++v) {
                texCoord01s.append(Vector4(G3DToOBJTex(geom->cpuVertexArray.vertex[v].texCoord0), 
                                           G3DToOBJTex(Vector2(geom->cpuVertexArray.texCoord1[v]))));
            }
            compressVertices<Vector4>(newTexMap, texCoord01s, baseTexIndex);
        } else if (hasTexCoord0) { 
            for (int v = 0; v < geom->cpuVertexArray.size(); ++v) {
                texCoord0s.append(G3DToOBJTex(geom->cpuVertexArray.vertex[v].texCoord0));
            }
            compressVertices<Vector2>(newTexMap, texCoord0s, baseTexIndex);   
        }
        texCoordMaps.append(newTexMap);

        //print out compressed lists of positions, normals, and texcoords
        for (int p = 0; p < positions.size(); ++p) {
            to.printf("%s %f %f %f \n","v", positions[p].x, positions[p].y, positions[p].z);
        }
        to.printf("%s %d %s","#", positions.size(), "vertices\n\n");

        for (int n = 0; n < normals.size(); ++n) {
            to.printf("%s %f %f %f \n","vn", normals[n].x, normals[n].y, normals[n].z);
        }
        to.printf("%s %d %s","#", normals.size(), "vertex normals\n\n");

        if (hasTexCoord1) {
            for (int t = 0; t < texCoord01s.size(); ++t) { 
                to.printf("%s %f %f %f %f\n","vt", texCoord01s[t].x, texCoord01s[t].y, texCoord01s[t].z, texCoord01s[t].w);
            }
        } else if (hasTexCoord0) {
            for (int t = 0; t < texCoord0s.size(); ++t) { 
                to.printf("%s %f %f \n","vt", texCoord0s[t].x, texCoord0s[t].y);
            }   
        }
        to.printf("%s %d %s","#", max(texCoord0s.size(), texCoord01s.size()), "texture coords\n\n");
    }

    // Print faces (index arrays)    
    for (int m = 0; m < m_meshArray.length(); ++m) {
        const Mesh* mesh = m_meshArray[m];

        // Obtain the index in m_geometryArray of the geometry used by the current mesh
        int geomIndex = 0;
        for (int j = 0; j < m_geometryArray.length(); ++j) {
            if (mesh->geometry == m_geometryArray[j]) {
                geomIndex = j;
            }
        }

        const int numIndices = mesh->cpuIndexArray.length();
        //print out all faces of the mesh, using the compressed index mappings

        to.printf("%s %s\n", "g", mesh->name.c_str());
        if (mesh->material->name() != "") {
            to.printf("%s %s\n", "usemtl", mesh->material->name().c_str());
        }
        for (int i = 0; i < numIndices; i += 3) {
            to.printf("%s ", "f");
            for (int k = 0; k < 3; ++k) {
                int rawIndex = mesh->cpuIndexArray[i + k];
                if (hasTexCoord0) {
                    to.printf(" %d/%d/%d", positionMaps[geomIndex].get(rawIndex), texCoordMaps[geomIndex].get(rawIndex), normalMaps[geomIndex].get(rawIndex));
                } else {
                    to.printf(" %d//%d", positionMaps[geomIndex].get(rawIndex), normalMaps[geomIndex].get(rawIndex));
                }
            }   
            to.writeNewline();
        }
        to.printf("%s %d %s","#", numIndices / 3, "faces\n\n");
    }
    to.commit();
}

} // namespace G3D
