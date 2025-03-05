#include "G3D-app/DDGIVolume.h"
#include "G3D-base/Any.h"
#include "G3D-base/AABox.h"

namespace G3D {

    DDGIVolumeSpecification::DDGIVolumeSpecification() {
    }


    Any DDGIVolumeSpecification::toAny() const {
        Any a(Any::TABLE, "DDGIVolumeSpecification");
        a["bounds"] = bounds;
        a["probeCounts"] = probeCounts;
        a["selfShadowBias"] = selfShadowBias;
        a["hysteresis"] = hysteresis;
        a["irradianceGamma"] = irradianceGamma;
        a["depthSharpness"] = depthSharpness;
        a["raysPerProbe"] = raysPerProbe;
        a["glossyToMatte"] = glossyToMatte;
        a["irradianceFormatIndex"] = irradianceFormatIndex;
        a["depthFormatIndex"] = depthFormatIndex;
        a["showLights"] = showLights;
        a["encloseBounds"] = encloseBounds;
        a["cameraLocked"] = cameraLocked;
        return a;
    }


    DDGIVolumeSpecification::DDGIVolumeSpecification
    (const Any& any,
        const AABox& defaultInscribedBounds,
        const AABox& defaultCircumscribedBounds) {

        *this = DDGIVolumeSpecification();
        AnyTableReader reader("DDGIVolumeSpecification", any);

        reader.getIfPresent("encloseBounds", encloseBounds);
        if (!reader.getIfPresent("bounds", bounds) || bounds.volume() == 0.0f) {
            bounds = encloseBounds ? defaultCircumscribedBounds : defaultInscribedBounds;
        }

        reader.getIfPresent("selfShadowBias", selfShadowBias);
        reader.getIfPresent("irradianceGamma", irradianceGamma);
        reader.getIfPresent("hysteresis", hysteresis);
        reader.getIfPresent("depthSharpness", depthSharpness);
        reader.getIfPresent("raysPerProbe", raysPerProbe);
        reader.getIfPresent("glossyToMatte", glossyToMatte);
        reader.getIfPresent("irradianceFormatIndex", irradianceFormatIndex);
        reader.getIfPresent("depthFormatIndex", depthFormatIndex);
        reader.getIfPresent("showLights", showLights);
        reader.getIfPresent("cameraLocked", cameraLocked);

        reader.getIfPresent("probeCounts", probeCounts);
        float maxProbeDistance;
        if (reader.getIfPresent("maxProbeDistance", maxProbeDistance)) {
            // Override explicit counts with a maximum distance
            probeCounts = Vector3int32(Vector3(bounds.high() - bounds.low()) / Vector3(maxProbeDistance, maxProbeDistance, maxProbeDistance));
            for (int i = 0; i < 3; ++i) {
                probeCounts[i] = ceilPow2(probeCounts[i]);
            }
            debugPrintf("Computed probeCounts = Vector3int32(%d, %d, %d)\n", probeCounts.x, probeCounts.y, probeCounts.z);
        }

        reader.verifyDone();

        // Do not go larger than 4k^2 (~67MB) texture, to fit in the max texture size for memory
        static const int MAX_TEXTURE_SIZE = square(4096);

        // Ensure that the number of probes fit in memory well. 
        // Assume the probe counts are powers of two.
        int totalProbes = probeCounts.x + probeCounts.y + probeCounts.z;
        while ((totalProbes * square(irradianceProbeResolution)) > MAX_TEXTURE_SIZE
            || (totalProbes * square(visibilityProbeResolution)) > MAX_TEXTURE_SIZE) {

            debugPrintf("Requested probe count is larger than max texture size of %d\n", MAX_TEXTURE_SIZE);
            // Heuristics: XZ resolution is probably more important than Y resolution,
            // unless Y resolution is relatively low...
            if (probeCounts.y > 8) {
                probeCounts.y /= 2;
            }
            else { probeCounts.x /= 2; probeCounts.z /= 2; }

            totalProbes = probeCounts.x + probeCounts.y + probeCounts.z;
        }
    }
}