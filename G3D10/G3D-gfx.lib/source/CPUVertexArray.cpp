/**
  \file G3D-gfx.lib/source/CPUVertexArray.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/CoordinateFrame.h"
#include "G3D-gfx/CPUVertexArray.h"
#include "G3D-gfx/AttributeArray.h"

namespace G3D {


void CPUVertexArray::Vertex::transformBy(const CoordinateFrame& cframe){
    position  = cframe.pointToWorldSpace(position);
    normal    = cframe.vectorToWorldSpace(normal);

    // The w component is just packed in.
    tangent   = Vector4(cframe.vectorToWorldSpace(tangent.xyz()), tangent.w);
}


CPUVertexArray::CPUVertexArray(const CPUVertexArray& otherArray) 
  : hasTexCoord0(otherArray.hasTexCoord0), 
    hasTexCoord1(otherArray.hasTexCoord1), 
    hasTangent(otherArray.hasTangent),
    hasVertexColors(otherArray.hasVertexColors) {
    
    vertex.copyPOD(otherArray.vertex);
    texCoord1.copyPOD(otherArray.texCoord1);
    vertexColors.copyPOD(otherArray.vertexColors);
    prevPosition.copyPOD(otherArray.prevPosition);
}


void CPUVertexArray::copyFrom(const CPUVertexArray& other){
    vertex.copyPOD(other.vertex);
    texCoord1.copyPOD(other.texCoord1);
    hasTexCoord0 = other.hasTexCoord0; 
    hasTexCoord1 = other.hasTexCoord1;
    hasTangent   = other.hasTangent;
    prevPosition = other.prevPosition;
    hasVertexColors = other.hasVertexColors;
}


void CPUVertexArray::transformAndAppend(const CPUVertexArray& otherArray, const CFrame& cframe){
    if (otherArray.hasTexCoord1) {
        texCoord1.appendPOD(otherArray.texCoord1);
    }

    const int oldSize = vertex.size();
    if (otherArray.hasVertexColors) {
        if (! hasVertexColors) {
            // ADD vertex colors for all of my previous vertices (i.e., for the *old* array)
            hasVertexColors = true;
            vertexColors.resize(vertex.size());
            for (int i = 0; i < vertexColors.size(); ++i) {
                vertexColors[i] = Color4::one();
            }
        }
        vertexColors.appendPOD(otherArray.vertexColors);
    } else if (hasVertexColors) {
        // Add bogus vertex colors for the *new* array
        vertexColors.resize(otherArray.size() + oldSize);
        for (int i = oldSize; i < vertexColors.size(); ++i) {
            vertexColors[i] = Color4::one();
        }
    }

    if ((hasPrevPosition() && otherArray.hasPrevPosition()) || (size() == 0 && otherArray.hasPrevPosition())) {
        prevPosition.appendPOD(otherArray.prevPosition);
        for (int i = oldSize; i < prevPosition.size(); ++i) {
            prevPosition[i] = cframe.pointToWorldSpace(prevPosition[i]);
        }
    } else {
        alwaysAssertM(! hasPrevPosition(), "Can't append a CPUVertexArray without prevPosition onto one with prevPosition");
    }

    vertex.appendPOD(otherArray.vertex);
    for (int i = oldSize; i < vertex.size(); ++i) {
        vertex[i].transformBy(cframe);
    }
}


void CPUVertexArray::transformAndAppend(const CPUVertexArray& otherArray, const CFrame& cframe, const CFrame& prevFrame) {
    if (otherArray.hasTexCoord1) {
        texCoord1.appendPOD(otherArray.texCoord1);
    }
    if (otherArray.hasVertexColors) {
        vertexColors.appendPOD(otherArray.vertexColors);
    }

    alwaysAssertM((size() == 0) || hasPrevPosition(), 
                  "Cannot invoke the three-argument transformAndAppend with hasPrevPosition() == false.");

    alwaysAssertM(! otherArray.hasPrevPosition(), 
                  "Cannot invoke the three-argument transformAndAppend with otherArray.hasPrevPosition() == true.");

    const int oldSize = vertex.size();

    vertex.appendPOD(otherArray.vertex);
    prevPosition.resize(vertex.size());
    for (int i = oldSize; i < vertex.size(); ++i) {
        prevPosition[i] = prevFrame.pointToWorldSpace(vertex[i].position);
        vertex[i].transformBy(cframe);
    }
}

void CPUVertexArray::copyToGPU
    (AttributeArray&               vertex, 
     AttributeArray&               normal, 
     AttributeArray&               packedTangent, 
     AttributeArray&               texCoord0,
     AttributeArray&               texCoord1,
     AttributeArray&               vertexColors,
     VertexBuffer::UsageHint       hint) const {
    AttributeArray ignore;
    copyToGPU(vertex, normal, packedTangent, texCoord0, texCoord1, vertexColors, ignore, ignore, hint);
}


void CPUVertexArray::copyToGPU
(AttributeArray&               vertexVR, 
 AttributeArray&               normalVR, 
 AttributeArray&               packedTangentVR, 
 AttributeArray&               texCoord0VR,
 AttributeArray&               texCoord1VR,
 AttributeArray&               vertexColorsVR,
 AttributeArray&               boneIndicesVR,
 AttributeArray&               boneWeightsVR,
 VertexBuffer::UsageHint       hint) const {

#   define OFFSET(field) ((size_t)(&dummy.field) - (size_t)&dummy)

    const int numVertices = size();
    if (numVertices > 0) {
        debugAssertM(! vertex[0].normal.isNaN(), "Tried to upload a CPUVertexArray to the GPU with a NaN normal");

        int cpuVertexByteSize = sizeof(Vertex) * numVertices;
        int texCoord1ByteSize = (hasTexCoord1 ? sizeof(Point2unorm16) * numVertices : 0);
        int vertexColorByteSize = (hasVertexColors ? sizeof(Color4) * numVertices : 0);
        int byteSize = cpuVertexByteSize;
        byteSize += texCoord1ByteSize;
        byteSize += vertexColorByteSize;
        byteSize += (hasBones     ? (sizeof(Point4int32) + sizeof(Vector4)) * numVertices : 0);

        // AttributeArray ensures four-byte alignment. Make the VertexBuffer with enough space to
        // account for the worst-case of three bytes added to each array.
        const int padding = 16;
        const shared_ptr<VertexBuffer>& buffer = VertexBuffer::create(byteSize + 2 * padding, VertexBuffer::WRITE_ONCE);
        
        AttributeArray all(byteSize, buffer);

        Vertex dummy;
        vertexVR        = AttributeArray(dummy.position,  numVertices, all, OFFSET(position),  (int)sizeof(Vertex));
        normalVR        = AttributeArray(dummy.normal,    numVertices, all, OFFSET(normal),    (int)sizeof(Vertex));
        packedTangentVR = AttributeArray(dummy.tangent,   numVertices, all, OFFSET(tangent),   (int)sizeof(Vertex));
        texCoord0VR     = AttributeArray(dummy.texCoord0, numVertices, all, OFFSET(texCoord0), (int)sizeof(Vertex));

        int consumedBytes = cpuVertexByteSize;

        if (hasTexCoord1) {
            texCoord1VR = AttributeArray(texCoord1, all, consumedBytes, (int)sizeof(Point2unorm16));
        } else {
            texCoord1VR = AttributeArray();
        }
        consumedBytes += texCoord1ByteSize;

        if (hasVertexColors) {
            vertexColorsVR = AttributeArray(vertexColors, all, consumedBytes, (int)sizeof(Color4));
        } else {
            vertexColorsVR = AttributeArray();
        }
        consumedBytes += vertexColorByteSize;


        if (hasBones) {
            boneIndicesVR = AttributeArray(boneIndices, all, consumedBytes, (int)sizeof(Vector4int32));
            consumedBytes += sizeof(Point4int32) * numVertices;
            boneWeightsVR = AttributeArray(boneWeights, all, consumedBytes, (int)sizeof(Vector4));
        } else {
            boneIndicesVR = AttributeArray();
            boneWeightsVR = AttributeArray();
        }

        // Copy all interleaved data at once
        Vertex* dst = (Vertex*)all.mapBuffer(GL_WRITE_ONLY);

        System::memcpy(dst, vertex.getCArray(), cpuVertexByteSize);

        all.unmapBuffer();
        dst = nullptr;
    }

#undef OFFSET
}

void CPUVertexArray::copyTexCoord0ToTexCoord1() {
    alwaysAssertM(hasTexCoord0, "Can't copy texCoord0 to texCoord1, since there are no texCoord0s");
    hasTexCoord1 = true;
    texCoord1.resize(vertex.size());
    for (int i = 0; i < vertex.size(); ++i) {
        texCoord1[i] = Point2unorm16(vertex[i].texCoord0);
    }
}

void CPUVertexArray::offsetAndScaleTexCoord1(const Point2& offset, const Point2& scale) {
    if ( hasTexCoord1 ) {
        for (int i = 0; i < texCoord1.size(); ++i) {
            texCoord1[i] = Point2unorm16( (Point2(texCoord1[i]) * scale) + offset);
        }
    }
}

} // G3D
