/**
  \file G3D-app.lib/include/G3D-app/HeightfieldModel.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_HeightfieldModel_h

#include "G3D-base/platform.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/Ray.h"
#include "G3D-app/UniversalMaterial.h"
#include "G3D-gfx/AttributeArray.h"
#include "G3D-app/Surface.h"
#include "G3D-app/Model.h"

namespace G3D {

class Any;
class Texture;
class Image;
class RenderDevice;
class AABox;
class Sphere;
class Shader;

/** 
  \brief A tiled regular heightfield with a single detail level, suitable for very large terrains observed mostly from above.

  The geometry is procedurally generated in the vertex shader, so this requires much less memory
  and can therefore represent much larger heightfields than an ArticulatedModel (which can also
  generate a heightfield at load time from an image).

  Restrictions of the current implementation:

  - Heightfields must be 8-bit
  - Tiles must be square (the heightfield can be a rectangle)
  - There must be an integer number of tiles in each dimension
  - The material must repeat at least once per tile (it will usually repeat far more often)
  
  To provide more interesting material properties that vary with elevation and angle, consider subclassing 
  HeightfieldModel or making a similar class from its source code.

  \sa ArticulatedModel
  */
class HeightfieldModel : public Model {
public:
    virtual const String& className() const override;

    void pose
    (Array<shared_ptr<Surface> >&   surfaceArray, 
     const CFrame&                  rootFrame,
     const CFrame&                  prevFrame,
     const shared_ptr<Entity>&      entity,
     const Model::Pose*             pose,
     const Model::Pose*             prevPose,
     const Surface::ExpressiveLightScatteringProperties& e) override;

    class Specification {
    public:
        /** The heightfield image, which must be convertible to R8 format. */
        String                  filename;

        /** Controls tiling resolution */
        int                     pixelsPerTileSide;

        /** Controls triangle tessellation */
        int                     pixelsPerQuadSide;

        /** Controls scale */
        float                   metersPerPixel;

        /** Material texture coordinate scale. The material texture coordinates tiles multiple times over the heightfield.*/
        float                   metersPerTexCoord;

        /** Maximum height in meters of the heightfield.  This multiplies the texture values. */
        float                   maxElevation;

        UniversalMaterial::Specification material;

        Specification();
        Specification(const Any& any);
    };

public:

    class Tile : public Surface {
    protected:
        const HeightfieldModel*         m_model;
        const shared_ptr<class Entity>  m_entity;
        const Point2int32               m_tileIndex;
        const CFrame                    m_frame;
        const CFrame                    m_previousFrame;

        void renderAll
        (RenderDevice*              rd, 
         const Array< shared_ptr< Surface > >& surfaceArray, 
         Args&                       args, 
         const shared_ptr<Shader>&   shader, 
         const CFrame&               previousCameraFrame,
         const Matrix4&              previousProjectionMatrix,
         bool                        bindPreviousMatrix,
         bool                        renderPreviousPosition         = false, 
         bool                        reverseOrder                   = false,
         const shared_ptr<Texture>&  previousDepthBuffer            = nullptr,
         const float                 minZSeparation                 = 0.0f, 
         bool                        renderTransmissiveSurfaces     = true) const;

    public:

        const HeightfieldModel* modelPtr() const {
            return m_model;
        }

        virtual void setStorage(ImageStorage newStorage) override;

        /** Allocates with System::malloc to avoid the performance
            overhead of creating lots of small heap objects using
            <code>::malloc</code>. */
        static void* operator new(size_t size) {
            return System::malloc(size);
        }

        static void operator delete(void* p) {
            System::free(p);
        }

        Tile(const HeightfieldModel* terrain, const Point2int32& tileIndex, const CFrame& frame, const CFrame& previousFrame, const shared_ptr<Entity>& entity, const Surface::ExpressiveLightScatteringProperties& expressiveLightScatteringProperties);

        virtual bool canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const override {
            return true;
        }

        virtual TransparencyType transparencyType() const override {
            return TransparencyType::NONE;
        }
 
        virtual void getCoordinateFrame (CoordinateFrame& cframe, bool previous=false) const override;

        virtual void getObjectSpaceBoundingBox (AABox& box, bool previous=false) const override;
 
        virtual void getObjectSpaceBoundingSphere (Sphere& sphere, bool previous=false) const override;
        
        virtual bool hasTransmission () const override;

        virtual String name () const override;

        virtual void render
        (RenderDevice*                           rd, 
         const LightingEnvironment&              environment,
         RenderPassType                          passType) const override;
        
        virtual void renderHomogeneous
        (RenderDevice*                           rd, 
         const Array< shared_ptr< Surface > >&   surfaceArray, 
         const LightingEnvironment&              environment,
         RenderPassType                          passType) const override;
        
        virtual void renderIntoGBufferHomogeneous
        (RenderDevice*                           rd, 
         const Array< shared_ptr< Surface > >&   surfaceArray, 
         const shared_ptr< GBuffer >&            gbuffer, 
         const shared_ptr<Texture>&              depthPeelTexture,
         const float                             minZSeparation,
         const LightingEnvironment&              lightingEnvironment) const override;
        
        virtual void renderWireframeHomogeneous(RenderDevice *rd, const Array< shared_ptr< Surface > > &surfaceArray, const Color4 &color, bool previous) const override;

        virtual void renderDepthOnlyHomogeneous
        (RenderDevice*                           rd, 
         const Array< shared_ptr<Surface> >&     surfaceArray, 
         const shared_ptr<Texture>&              depthPeelTexture,
         const float                             depthPeelEpsilon,
         TransparencyTestMode                    transparencyTestMode,  
         const Color3&                           transmissionWeight) const override;
    };

protected:

    const Specification         m_specification;

    const String                m_name;

    int                         m_quadsPerTileSide;

    /** Shared vertex buffer for the entire mesh. Stored in XY, since the mesh is flat, with 
        unit spacing between vertices (i.e., vertices are at integer positions) */
    AttributeArray              m_positionArray;

    /** Indices of the mesh. */
    IndexStream                 m_indexStream;

    /** Used for all normal rendering. */
    shared_ptr<Shader>          m_shader;

    shared_ptr<Shader>          m_gbufferShader;

    /** Used for depth-only and wire-frame rendering */
    shared_ptr<Shader>          m_depthAndColorShader;

    shared_ptr<UniversalMaterial> m_material;

    /** Elevation texture */
    shared_ptr<Texture>         m_elevation;

    /** Elevation image */
    shared_ptr<Image>           m_elevationImage;

    HeightfieldModel(const Specification& spec, const String& name);

    /** Called from the constructor */
    void loadShaders();

    /** Called from the constructor */
    void generateGeometry();

    /** This binds attribute arrays, so it cannot accept a UniformTable argument. */
    void setShaderArgs(Args& args) const;

public:

    static shared_ptr<HeightfieldModel> create(const Specification& spec, const String& name = "Heightfield") {
        return createShared<HeightfieldModel>(spec, name);
    }


    static lazy_ptr<Model> lazyCreate(const Specification& s, const String& name = "") {
        return lazy_ptr<Model>( [s, name] { return HeightfieldModel::create(s, name); } );
    }

    /** \sa G3D::Scene::registerModelSubclass */
    static lazy_ptr<Model> lazyCreate(const String& name, const Any& any) {
        return lazyCreate(any, name);
    }


    virtual const String& name() const override {
        return m_name;
    }

    const shared_ptr<Texture> elevationTexture() const {
        return m_elevation;
    }

    const shared_ptr<Image> elevationImage() const {
        return m_elevationImage;
    }

    const Specification& specification() const {
        return m_specification;
    }

    virtual bool intersect
    (const Ray&                      ray, 
     const CoordinateFrame&          cframe, 
     float&                          maxDistance, 
     Model::HitInfo&                 info           = Model::HitInfo::ignore,
     const Entity*                   entity         = nullptr,
     const Model::Pose*              pose           = nullptr) const override;

    /** 
      Return the elevation (y value) under <code>(osPoint.x, -, osPoint.z)</code> according to the tessellation
      used for rendering (i.e., using barycentric interpolation on the triangles, not bilinear interpolation on the grid).

      The faceNormal is the normal to the triangle, not the shading normal.
    */
    float elevation(const Point3& osPoint, Vector3& faceNormal) const;

    float elevation(const Point3& osPoint) const {
        Vector3 ignore;
        return elevation(osPoint, ignore);
    }
};

} // namespace G3D
