/**
  \file G3D-app.lib/include/G3D-app/DDGIVolume.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2020, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/Array.h"
#include "G3D-gfx/Texture.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-app/GFont.h"
#include "G3D-gfx/AttributeArray.h"
#include "G3D-app/Scene.h"
#include "G3D-app/debugDraw.h"
#include "G3D-app/DDGIVolumeSpecification.h"

namespace G3D {


	G3D_DECLARE_ENUM_CLASS(DebugVisualizationMode,
		PROBE_WEIGHTS,
		NONE);

	/**
	  A volume of space within which irradiance queries at arbitrary points are supported
	  using a grid of DDGI probes. A single DDGIVolume may cover the entire scene and
	  use multiple update cascades within it.

	  If there are parts of your scene with very different geometric density or dimensions,
	  you can have multiple DDGIVolumes.
	*/
	class DDGIVolume : public ReferenceCountedObject {

	public:

		enum ProbeStates {
			OFF = 0,
			ASLEEP = 1,
			JUST_WOKE = 2,
			AWAKE = 3,
			JUST_VIGILANT = 4,
			VIGILANT = 5,
			UNINITIALIZED = 6
		};

	protected:

		String                                      m_name;

		/** Irradiance encoded with a high gamma curve */
		shared_ptr<Texture>                         m_irradianceTexture;

		/** R = mean distance, G = mean distance^2 */
		shared_ptr<Texture>                         m_visibilityTexture;

		shared_ptr<Texture>                         m_probeOffsetTexture;
		shared_ptr<GLPixelTransferBuffer>           m_sleepingProbesBuffer;

		Point3                                      m_probeGridOrigin;
		Vector3                                     m_probeSpacing;

		Vector3int32                                m_phaseOffset = Vector3int32(0, 0, 0);

		// Bitmask (in vector form) to determine which plane should be set to uninitialized
		// for a camera-locked volume. These are the "new" probes.
		Vector3int32                                m_uninitializedPlane = Vector3int32(0, 0, 0);

		/** If true, set hysteresis to zero and force all probes to re-render.
			Used for when parameters change. */
		bool                                        m_firstFrame = true;

		/** When lighting changes dramatically, we drop the hysteresis to 50% of the typical value
			for a few frames.  */
		int                                         m_lowIrradianceHysteresisFrames = 0;
		int                                         m_reducedIrradianceHysteresisFrames = 0;

		int                                         m_lowVisibilityHysteresisFrames = 0;

		/** Maximum distance that can be written to a probe. */
		float                                       m_maxDistance = 4.0f;

		shared_ptr<GLPixelTransferBuffer>			m_conservativeAABoundsPBO;

		bool                                        m_probeSleeping;

		///////////////////////////////////////////////////////////////////////////////////
		//
		// Helpful GUI parameters
		static const Array<const ImageFormat*>      s_guiIrradianceFormats;
		static const Array<const ImageFormat*>      s_guiDepthFormats;

		int                                         m_guiIrradianceFormatIndex = 3;
		int                                         m_guiVisibilityFormatIndex = 2;
		bool                                        m_guiProbeFormatChanged;

		static shared_ptr<GFont>                    s_guiLabelFont;

		// For probe visualization
		static AttributeArray                       s_debugProbeVisualizationVertexArray;
		static IndexStream                          s_debugProbeVisualizationIndexStream;
		static const Array<Color3>					s_debugProbeVisualizationColors;
		static int									s_debugProbeVisualizationColorsIndex;
		static bool									s_visualizeDebugColors;

		int											m_debugProbeVisualizationColorsIndex;

		Point3 probeIndexToPosition(int index) const;

		Point3int32 probeIndexToGridIndex(int index) const;

		void setProbeStatesToUninitialized();

		DDGIVolumeSpecification                               m_specification;

		/** Stored so that we can use the *same* random orientation
		for each of many calls that recompute the ray direction. */
		Matrix3                                     m_randomOrientation;

		/** 1D mapping from probe indices to ray block indices in the compressed ray buffers. */
		shared_ptr<GLPixelTransferBuffer>           m_rayBlockIndexOffset;

		int m_skippedProbes = 0;

	public:

		const shared_ptr<GLPixelTransferBuffer>& rayBlockIndexOffsetBuffer() {
			return m_rayBlockIndexOffset;
		}

		int skippedProbes() {
			return m_skippedProbes;
		}

		virtual const Matrix3& randomOrientation() {
			return m_randomOrientation;
		}

		void setRandomOrientation(const Matrix3& orientation) {
			m_randomOrientation = orientation;
		}

		void computeProbeOffsetsAndFlags(RenderDevice* rd,
			const shared_ptr<Texture>& rayHitLocations,
			const int						offset,
			const int                      raysPerProbe,
			bool                           adjustOffsets = true);


		/** Update both irradiance and depth using newly sampled rays.
		Optional offset within the ray textures for multiple volumes.
		For now, this offset is equal to the number of probes in the
		first volume: the texture dimensions are:

		w = max(ddgiVolumePrimary.raysPerProbe, ddgiVolumeSecondary.raysPerProbe);
		h = ddgiVolumePrimary.numProbes + ddgiVolumeSecondary.numProbes;

		Lot's of unused texture space in the common case where both volumes
		cast a different number of rays/probe. However, we'll clean this up
		by compacting the smaller buffer to have multiple probes on one row.
		*/
		virtual void updateAllProbeTypes
		(RenderDevice* rd,
			const Array<shared_ptr<Surface>>& surfaceArray,
			const shared_ptr<Texture>& rayHitLocations,
			const shared_ptr<Texture>& rayHitRadiance,
			const int                                   offset,
			const int                                   raysPerProbe);


		DDGIVolumeSpecification& specification() {
			return m_specification;
		}

		void init(const String& name, const DDGIVolumeSpecification& spec, const Point3& cameraPos = Point3(0, 0, 0));


		// Allow apps to update the volume data without 
		// using GIRenderer::updateDiffuseGI().
		void updateIrradianceTexture(const shared_ptr<Texture>& newTexture);
		void updateIrradianceTexture(const shared_ptr<GLPixelTransferBuffer>& pbo);

		void updateVisibilityTexture(const shared_ptr<Texture>& newTexture);
		void updateVisibilityTexture(const shared_ptr<GLPixelTransferBuffer>& pbo);

		void updateProbeOffsetTexture(const shared_ptr<Texture>& newTexture);
		void updateProbeOffsetTexture(const shared_ptr<GLPixelTransferBuffer>& pbo);


		static void setVisualizeDebugColors(bool b) {
			s_visualizeDebugColors = b;
		}

		static const bool visualizeDebugColors() {
			return s_visualizeDebugColors;
		}

		static void loadGeometry
		(const String& filename,
			float                                       scale,
			Color3                                      faceColor,
			AttributeArray& vertexArray,
			IndexStream& indexStream);

		static shared_ptr<DDGIVolume> create(const String& name, const DDGIVolumeSpecification& spec, const Point3& cameraPos);

		virtual void resizeIfNeeded();

		void setShaderArgs(UniformTable& args, const String& prefix);

		// Unifies code instead of repeating it in a number of places.
		// Public to expose this functionality to the App for profiling purposes.
		Vector4int8* mapSleepingProbesBuffer(bool forWriting = false);
		void         unmapSleepingProbesBuffer();

		const shared_ptr<Texture>& irradianceTexture() {
			return m_irradianceTexture;
		}

		const shared_ptr<Texture>& visibilityTexture() {
			return m_visibilityTexture;
		}

		const shared_ptr<Texture>& probeOffsetTexture() {
			return m_probeOffsetTexture;
		}

		void setFirstFrame(bool b) {
			m_firstFrame = b;
		}
		void setCameraLocked(bool b) {
			m_specification.cameraLocked = b;
		}

		bool cameraLocked() {
			return m_specification.cameraLocked;
		}

		int guiIrradianceFormatIndex() {
			return m_guiIrradianceFormatIndex;
		}
		void setGuiIrradianceFormatIndex(int index) {
			m_guiIrradianceFormatIndex = index;
		}

		int guiVisibilityFormatIndex() {
			return m_guiVisibilityFormatIndex;
		}
		void setGuiVisibilityFormatIndex(int index) {
			m_guiVisibilityFormatIndex = index;
		}

		void setProbeSleeping(bool b) {
			m_probeSleeping = b;
		}

		bool probeSleeping() {
			return m_probeSleeping;
		}

		/** Should the bounds of the probes go around the geo or to the edge of the geo? */
		bool encloseScene() {
			return m_specification.encloseBounds;
		}

		void setEncloseScene(bool b) {
			m_specification.encloseBounds = b;
		}

		const int irradianceOctSideLength() {
			return m_specification.irradianceProbeResolution;
		}

		const int visibilityOctSideLength() {
			return m_specification.visibilityProbeResolution;
		}

		void setIrradianceOctSideLength(int sideLengthSize, RenderDevice* rd) {
			m_specification.irradianceProbeResolution = sideLengthSize;
		}

		void setDepthOctSideLength(int sideLengthSize, RenderDevice* rd) {
			m_specification.visibilityProbeResolution = sideLengthSize;
		}

		void debugDrawProbeLabels(float probeVisualizationRadius) const;

		void debugRenderProbeVisualization(RenderDevice* rd, const shared_ptr<Camera>& camera, const bool visualizeDepth, const float probeVisualizationRadius);

		const ImageFormat* irradianceFormat() {
			return s_guiIrradianceFormats[m_guiIrradianceFormatIndex];
		}

		static const Texture::Encoding& normalEncoding() {
			static Texture::Encoding enc(ImageFormat::RG8(), FrameName::WORLD, 2.0f, -1.0f);
			return enc;
		}

		int probeCount() const {
			return m_specification.probeCounts.x * m_specification.probeCounts.y * m_specification.probeCounts.z;
		}

		const Vector3int32& probeCounts() const {
			return m_specification.probeCounts;
		}

		/** Switch to low hysteresis for a few frames */
		void onGlobalLightChange() {
			m_lowIrradianceHysteresisFrames = 10;
		}

		void onLargeObjectChange() {
			if (!m_specification.detectLargeObjectMotion) {
				return;
			}
			m_lowVisibilityHysteresisFrames = 7;
			onGlobalLightChange();
		}

		void onSmallLightChange() {
			m_reducedIrradianceHysteresisFrames = 4;
		}

		bool hasTracingProbes() {
			return m_skippedProbes != probeCount();
		}

		void gatherTracingProbes(const Array<ProbeStates>& states);
		void notifyOfDynamicObjects(const Array<AABox>& currentBoxArray, const Array<Vector3>& velocityArray);
		bool notifyOfCameraPosition(const Point3& cameraWSPosition);
	};

}