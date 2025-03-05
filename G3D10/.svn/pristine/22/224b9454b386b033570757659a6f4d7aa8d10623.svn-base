/**
  \file G3D-app.lib/include/G3D-app/DirectionHistogram.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#ifndef GLG3D_DirectionHistogram_h
#define GLG3D_DirectionHistogram_h

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/Color3.h"
#include "G3D-base/Color4.h"
#include "G3D-gfx/VertexBuffer.h"
#include "G3D-gfx/AttributeArray.h"
#include "G3D-app/TriTree.h"
#include "G3D-app/Material.h"
#include "G3D-app/SlowMesh.h"

namespace G3D {

class RenderDevice;
    
/** \brief A histogram on the surface of a sphere.
    Useful for visualizing BSDFs. 
    
    The histogram drawn is a smoothing of the actual distribution
    by a \f$ \cos^{sharp} \f$ filter to ensure that it is not 
    undersampled by the underlying histogram mesh and buckets.

    Storage size is constant in the amount of data.  Input is 
    immediately inserted into a bucket and then discarded.
  */
class DirectionHistogram : ReferenceCountedObject {
private:

    int                 m_slices;

    /** Vertices of the visualization mesh, on the unit sphere. */
    Array<Point3>       m_meshVertex;

    /** Indices into meshVertex of the trilist for the visualization mesh. */
    Array<int>          m_meshIndex;

    /** Histogram buckets.  These are the scales of the corresponding m_meshVertex.*/
    Array<float>        m_bucket;

    /** Used to quickly find the quad.  The Tri::data field is the pointer 
       (into a subarray of m_meshIndex) of the four vertices of the quad hit.*/
    shared_ptr<TriTree> m_tree;

    /** m_area[i] = inverse of the sum of the areas adjacent to vertex[i] */
    Array<float>        m_invArea;

    /** True when the AttributeArray needs to be recomputed */
    bool                m_dirty;

    float               m_sharp;

    int                 m_numSamples;

    class VertexIndexIndex : public ReferenceCountedObject {
    public:
        int index;
        VertexIndexIndex(int i) : index(i) {}
    };

    /** Volume of a tetrahedron whose 4th vertex is at the origin.  
        The vertices are assumed to be
        in ccw order.*/
    static float tetrahedronVolume(const Vector3& v0, const Vector3& v1, const Vector3& v2);

    /** Compute the total volume of the distribution */
    float totalVolume() const;

    /** Assumes vector has unit length.

       \param startIndex Inclusive
       \param stopIndex  Inclusive
    */
    void insert(const Vector3& vector, float weight, int startIndex, int stopIndex);

public:

    /**
       \param axis place histogram buckets relative to this axis

       \param numSlices Number of lat and long slices to make.*/
    DirectionHistogram(int numSlices = 50, const Vector3& axis = Vector3::unitZ());

    /** Discard all data */
    void reset();

    /**
     \brief Insert a new data point into the set.
      Only the direction of @a vector matters; it will be normalized.
     */
    void insert(const Vector3& vector, float weight = 1.0f);
    void insert(const Array<Vector3>& vector, const Array<float>& weight);

    /** Draw a wireframe of the distribution.  Renders with approximately constant volume. */
    void render(
        class RenderDevice* rd, 
        const Color3& solidColor = Color3::white(), 
        const Color4& lineColor = Color3::black());

};

} // G3D

#endif
