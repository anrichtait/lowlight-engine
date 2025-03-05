/**
  \file G3D-app.lib/include/G3D-app/DDGIVolumeSpecification.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2020, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#include "G3D-base/platform.h"
#include "G3D-base/AABox.h"
namespace G3D {
	/**
	  A volume of space within which irradiance querries at arbitrary points are supported
	  using a grid of DDGI probes. A single DDGIVolume may cover the entire scene and
	  use multiple update cascades within it.

	  If there are parts of your scene with very different geometric density or dimensions,
	  you can have multiple DDGIVolumes.
	*/
	class DDGIVolumeSpecification {
	public:
		/** Bounding box on the volume */
		AABox           bounds = AABox(Point3(0.0f, 0.0f, 0.0f), Point3(0.0f, 0.0f, 0.0f));

		/** Number of probes on each axis within the volume. */
		//Vector3int32    probeCounts = Vector3int32(32, 4, 32);
		Vector3int32    probeCounts = Vector3int32(8, 4, 8);

		/** Side length of one face of the probe in the texture map, not including a 1-pixel border on each side. */
		int             irradianceProbeResolution = 8;

		/** Side length of one face of the probe in the texture map, not including a 1-pixel border on each side. */
		int             visibilityProbeResolution = 16;

		/** Slightly bump the location of the shadow test point away from the shadow casting surface.
			The shadow casting surface is the boundary for shadow, so the nearer an imprecise value is
			to it the more the light leaks.

			This is roughly in the scale of world units, although it is applied to both normals and
			view vectors so acts more like 2x the magnitude.
		*/
		float           selfShadowBias = 0.3f;

		/** Control the weight of new rays when updating each irradiance probe. A value close to 1 will
			very slowly change the probe textures, improving stability but reducing accuracy when objects
			move in the scene, while values closer to 0.9 or lower will rapidly react to scene changes
			but exhibit flickering.
		*/
		float           hysteresis = 0.98f;

		/** Exponent for depth testing. A high value will rapidly react to depth discontinuities, but risks
			exhibiting banding.
		*/
		float           depthSharpness = 50.0f;

		/** Number of rays emitted each frame for each probe in the scene. This is independent
			of the resolution of the probes. */
		int             raysPerProbe = 256;

		/** If true, add the glossy coefficient in to matte term for a single albedo. Eliminates low-probability,
			temporally insensitive caustic effects. */
		bool            glossyToMatte = true;

		/** We blend in post-tone map space... */
		float           irradianceGamma = 5.0f;

		int             irradianceFormatIndex = 2;
		int             depthFormatIndex = 1;

		bool            showLights = false;
		bool            encloseBounds = true;

		bool            enableProbeUpdate = true;
		bool            enableSecondOrderGlossy = true;

		bool            detectLargeObjectMotion = true;

		float           probeOffsetLimit = 0.5f;

		bool            cameraLocked = false;

		DDGIVolumeSpecification();

		Any toAny() const;

		DDGIVolumeSpecification(
			const Any&,
			const AABox& defaultInscribedBounds,
			const AABox& defaultCircumscribedBounds);

		DDGIVolumeSpecification(const Any& any) {
			*this = DDGIVolumeSpecification(any, AABox(Vector3(0, 0, 0), Vector3(0, 0, 0)), AABox(Vector3(0, 0, 0), Vector3(0,0,0)));
		}
	};

}