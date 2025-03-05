/**
  \file G3D-app.lib/include/G3D-app/TextSurface.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/


#pragma once
#define GLG3D_TextSurface_h
#include "G3D-app/FontModel.h"
#include "G3D-app/Surface.h"
#include "G3D-base/AABox.h"
namespace G3D {
    /**
    \brief A billboard displaying text.
    */
    class TextSurface : public Surface {
    protected:

        const String                                m_name;
        const CFrame                                m_frame;
        const CFrame                                m_previousFrame;
        const shared_ptr<FontModel>                 m_fontModel;
        const String                                m_profilerHint;

        TextSurface
        (const String&                                       name,
            const CFrame&                                       frame,
            const CFrame&                                       previousFrame,
            const shared_ptr<FontModel>&                        model,
            const shared_ptr<Entity>&                           entity,
            const Surface::ExpressiveLightScatteringProperties& expressive);

    public:

        virtual TransparencyType transparencyType() const override {
            return TransparencyType::NONE;
        }

        virtual void renderHomogeneous
        (RenderDevice*                        rd,
            const Array<shared_ptr<Surface> >&   surfaceArray,
            const LightingEnvironment&           lightingEnvironment,
            RenderPassType                       passType) const override {
            for (int i = 0; i < surfaceArray.size(); ++i) {
                surfaceArray[i]->render(rd, lightingEnvironment, passType);
            }
        }

        virtual void pose
        (Array<shared_ptr<Surface> >&   surfaceArray,
            const CFrame&                  rootFrame,
            const CFrame&                  prevFrame,
            const shared_ptr<Entity>&      entity,
            const Model::Pose*             pose) {}

        static shared_ptr<TextSurface> create
        (const String&                                       name,
            const CFrame&                                       frame,
            const CFrame&                                       previousFrame,
            const shared_ptr<FontModel>&                        model,
            const shared_ptr<Entity>&                           entity,
            const Surface::ExpressiveLightScatteringProperties& expressive);

        virtual bool canBeFullyRepresentedInGBuffer(const GBuffer::Specification& specification) const override {
            return false;
        }

        virtual CoordinateFrame frame(bool previous = false) const override {
            if (previous) {
                return m_previousFrame;
            }
            else {
                return m_frame;
            }
        }

        //This will call Draw2D, Draw3D
        virtual void render(RenderDevice *rd, const LightingEnvironment &environment, RenderPassType passType) const override;

        virtual void setStorage(ImageStorage newStorage) override {}

        /** Renders into depthOnly buffer for shadow mapping */
        virtual void renderDepthOnlyHomogeneous
        (RenderDevice*                                   rd,
            const Array<shared_ptr<Surface> >&              surfaceArray,
            const shared_ptr<Texture>&                      previousDepthBuffer,
            const float                                     minZSeparation,
            TransparencyTestMode                            transparencyTestMode,
            const Color3&                                   transmissionWeight) const override {}

        /** Renders into GBuffer for deferred shading */
        /*virtual void renderIntoGBufferHomogeneous
        (RenderDevice*                                   rd,
            const Array< shared_ptr< Surface > > &          surfaceArray,
            const shared_ptr< GBuffer > &                   gbuffer,
            const CFrame &                                  previousCameraFrame,
            const CFrame &                                  expressivePreviousCameraFrame,
            const shared_ptr< Texture > &                   depthPeelTexture,
            const float                                     minZSeparation,
            const LightingEnvironment &                     lighting) const {}
    */
        /** Intentionally does nothing */
        virtual void renderIntoSVOHomogeneous(RenderDevice *rd, Array< shared_ptr< Surface > > &surfaceArray, const shared_ptr< SVO > &svo, const CFrame &previousCameraFrame) const override {}

        /** Intentionally does nothing */
        virtual void renderWireframeHomogeneous(RenderDevice *rd, const Array< shared_ptr< Surface > > &surfaceArray, const Color4 &color, bool previous) const override {}

        virtual void getObjectSpaceBoundingBox(G3D::AABox& bounds, bool previous) const override {
            Vector2 size2D = m_fontModel->font->bounds(m_fontModel->modelPose()->text, m_fontModel->modelPose()->size, GFont::PROPORTIONAL_SPACING);
            //Because the text always faces the camera, this bound is conservative to include all possible text orientations.
            //TODO: restore to y
            bounds = AABox(Point3(-size2D.x * 0.5f, -size2D.y * 0.5f, -size2D.x * 0.5f), Point3(size2D.x * 0.5f, size2D.y * 0.5f, size2D.x * 0.5f));
        }

        virtual void getObjectSpaceBoundingSphere(G3D::Sphere& bounds, bool previous) const override {
            Vector2 size2D = m_fontModel->font->bounds(m_fontModel->modelPose()->text, m_fontModel->modelPose()->size, GFont::PROPORTIONAL_SPACING);
            bounds = Sphere(sqrt(size2D.x * size2D.x + size2D.y * size2D.y));
        }

    }; //class TextSurface
} //namespace G3D
