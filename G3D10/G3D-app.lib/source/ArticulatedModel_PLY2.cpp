/**
  \file G3D-app.lib/source/ArticulatedModel_PLY2.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/


#include "G3D-app/ArticulatedModel.h"
#ifndef DISABLE_PLY2
#include "G3D-base/FileSystem.h"

namespace G3D {

// There is no "ParsePLY2" because PLY2 parsing is trivial--it has no subparts or materials,
// and is directly an indexed format.
void ArticulatedModel::loadPLY2(const Specification& specification) {
    
    Part*       part = addPart(m_name);
    Geometry*   geom = addGeometry("geom");
    Mesh*       mesh = addMesh("mesh", part, geom);
    mesh->material = UniversalMaterial::create();
    
    TextInput ti(specification.filename);
        
    const int nV = iFloor(ti.readNumber());
    const int nF = iFloor(ti.readNumber());
    
    geom->cpuVertexArray.vertex.resize(nV);
    mesh->cpuIndexArray.resize(3 * nF);
    geom->cpuVertexArray.hasTangent = false;
    geom->cpuVertexArray.hasTexCoord0 = false;
    
        
    for (int i = 0; i < nV; ++i) {
        geom->cpuVertexArray.vertex[i].normal = Vector3::nan();
        Vector3& v = geom->cpuVertexArray.vertex[i].position;
        for (int a = 0; a < 3; ++a) {
            v[a] = float(ti.readNumber());
        }
    }
                
    for (int i = 0; i < nF; ++i) {
        const int three = ti.readInteger();
        alwaysAssertM(three == 3, "Ill-formed PLY2 file");
        for (int j = 0; j < 3; ++j) {
            mesh->cpuIndexArray[3*i + j] = ti.readInteger();
        }
    }
}

} // namespace G3D
#endif //DISABLE_PLY2
