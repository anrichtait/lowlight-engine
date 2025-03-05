/**
  \file G3D-app.lib/source/DDGIVolume.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2020, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/DDGIVolume.h"
#include "G3D-gfx/Shader.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/Camera.h"
#include "G3D-app/GuiText.h"
#include "G3D-app/Skybox.h"


namespace G3D {

shared_ptr<GFont> DDGIVolume::s_guiLabelFont;
AttributeArray    DDGIVolume::s_debugProbeVisualizationVertexArray;
IndexStream       DDGIVolume::s_debugProbeVisualizationIndexStream;
int		  DDGIVolume::s_debugProbeVisualizationColorsIndex = 0;
bool		  DDGIVolume::s_visualizeDebugColors = false;

const Array<Color3> DDGIVolume::s_debugProbeVisualizationColors = {
    Color3::red(),
    Color3::green(),
    Color3::blue()
};

// These must be 4 channel to satisfy binding as image uniforms.
const Array<const ImageFormat*> DDGIVolume::s_guiIrradianceFormats = {
    ImageFormat::RGB5A1(),
    ImageFormat::RGBA8(),
    ImageFormat::RGB10A2(),
    ImageFormat::R11G11B10F(),
    ImageFormat::RGBA16F(),
    ImageFormat::RGBA32F()
};
    
const Array<const ImageFormat*> DDGIVolume::s_guiDepthFormats = {
    ImageFormat::RG8(),
    ImageFormat::RG16F(),
    ImageFormat::RG32F()
};


shared_ptr<DDGIVolume> DDGIVolume::create(const String& name, const DDGIVolumeSpecification& spec, const Point3& cameraPos) {
    const shared_ptr<DDGIVolume>& ddgiVolume = createShared<DDGIVolume>();
    ddgiVolume->init(name, spec, cameraPos);
    return ddgiVolume;
}

void DDGIVolume::updateIrradianceTexture(const shared_ptr<Texture>& newTexture) {
    newTexture->copyInto(m_irradianceTexture);
}
    
void DDGIVolume::updateVisibilityTexture(const shared_ptr<Texture>& newTexture) {
    newTexture->copyInto(m_visibilityTexture);
}
    
void DDGIVolume::updateProbeOffsetTexture(const shared_ptr<Texture>& newTexture) {
    newTexture->copyInto(m_probeOffsetTexture);
}

    void DDGIVolume::updateIrradianceTexture(const shared_ptr<GLPixelTransferBuffer>& pbo) {
        m_irradianceTexture->update(pbo);
    }
    
    void DDGIVolume::updateVisibilityTexture(const shared_ptr<GLPixelTransferBuffer>& pbo) {
        m_visibilityTexture->update(pbo);
    }
    
    void DDGIVolume::updateProbeOffsetTexture(const shared_ptr<GLPixelTransferBuffer>& pbo) {
        m_probeOffsetTexture->update(pbo);
    }

    void DDGIVolume::init(const String& name, const DDGIVolumeSpecification& spec, const Point3& cameraPos) {
        m_name = name;

        m_specification = spec;
        alwaysAssertM(G3D::isPow2(m_specification.probeCounts.x * m_specification.probeCounts.y * m_specification.probeCounts.z),
            "Probe count must be power of two");

        const Point3& lo = spec.bounds.low();
        const Point3& hi = spec.bounds.high();
        m_probeSpacing = (hi - lo) / (Vector3(m_specification.probeCounts) - Vector3(1, 1, 1)).max(Vector3(1, 1, 1));
        m_probeGridOrigin = lo;

        // Center volume on camera if camera locked
        if (spec.cameraLocked) {
            m_probeGridOrigin += cameraPos;
        }

        m_guiIrradianceFormatIndex = spec.irradianceFormatIndex;
        m_guiVisibilityFormatIndex = spec.depthFormatIndex;

        // Set the visualization index for this volume and increment the global index.
        m_debugProbeVisualizationColorsIndex = s_debugProbeVisualizationColorsIndex;
        s_debugProbeVisualizationColorsIndex = (s_debugProbeVisualizationColorsIndex + 1) % s_debugProbeVisualizationColors.size();

        setProbeSleeping(true);

            // Special case of 1-probe high surface
        for (int i = 0; i < 3; ++i) {
            if (m_specification.probeCounts[i] == 1) {
                m_probeGridOrigin[i] = (hi[i] + lo[i]) / 2.0f;
            }
        }

        // Load geometry for probe positions
        static bool firstInstance = true;
        if (firstInstance) {
            // Scaling is applied in the vertex shader.
            loadGeometry(System::findDataFile("model/sphere/sphere.zip/sphere-cylcoords-1k.obj"), 1.0f, Color3(1), s_debugProbeVisualizationVertexArray, s_debugProbeVisualizationIndexStream);
            s_guiLabelFont = GFont::fromFile(System::findDataFile("eurostyle.fnt"));
            firstInstance = false;
        }

        // Round up to the next multiple of 32 rays/probe. You should try to prevent raysPerProbe from ever being set
        // to something that isn't 32 (lowest end), or a multiple of 64 (better perf).
        m_specification.raysPerProbe = max(32, 32 * iRound(m_specification.raysPerProbe / 32.0f));

        m_guiProbeFormatChanged = true;

        m_firstFrame = true;

        const Vector3 boundingBoxLengths(spec.bounds.high() - spec.bounds.low());
        // Slightly larger than the diagonal across the grid cell
        m_maxDistance = (boundingBoxLengths / spec.probeCounts).length() * 1.5f;
    }

    void DDGIVolume::setProbeStatesToUninitialized() {
        const shared_ptr<GLPixelTransferBuffer>& temp = GLPixelTransferBuffer::create(m_probeOffsetTexture->width(), m_probeOffsetTexture->height(), m_probeOffsetTexture->format());

        Vector4int8* tempPtr = (Vector4int8*)temp->mapWrite();
        for (int i = 0; i < m_probeOffsetTexture->width() * m_probeOffsetTexture->height(); ++i) {
            // To turn probe sleeping (and the position optimizer) off, simply make all probes vigilant
            //tempPtr[i] = Vector4int8(0, 0, 0, DDGIVolume::ProbeStates::VIGILANT);
            tempPtr[i] = Vector4int8(0, 0, 0, DDGIVolume::ProbeStates::UNINITIALIZED);
        }

        temp->unmap();
        tempPtr = nullptr;

        m_probeOffsetTexture->update(temp);
    }


    void DDGIVolume::computeProbeOffsetsAndFlags(RenderDevice* rd,
        const shared_ptr<Texture>& rayHitLocations,
        const int                                  offset,
        const int                                  raysPerProbe,
        bool                                       adjustOffsets) {

        Args args;
        m_rayBlockIndexOffset->bindAsShaderStorageBuffer(2);

        args.setRect(m_probeOffsetTexture->rect2DBounds());
        args.setMacro("RAYS_PER_PROBE", raysPerProbe);
        args.setUniform("randomOrientation", m_randomOrientation);
        setShaderArgs(args, "ddgiVolume.");
        args.setUniform("maxDistance", m_maxDistance);

        args.setUniform("offset", offset);

        args.setMacro("ADJUST_OFFSETS", adjustOffsets);

        rayHitLocations->setShaderArgs(args, "rayHitLocations.", Sampler::buffer());

        const Vector3int32& p = probeCounts();

        // Assume we never go below 8 probes in a horizontal slice
        assert(p.x * p.y % 8 == 0);
        assert(p.z % 4 == 0);

        // Using 8x4 threadblocks for perfect coherency over probe dimensions 
        // at 32 threads/block.
        const Vector2int32 blockDims(8, 4);

        args.setComputeGridDim(Vector3int32(p.x * p.y / blockDims.x, p.z / blockDims.y, 1));
        args.setComputeGroupSize(Vector3int32(blockDims, 1));


        LAUNCH_SHADER("DDGIVolume_computeProbeOffsets.glc", args);
        debugAssertGLOk();

        m_firstFrame = false;
    }

#if 0
    static Color3 desertGradientdesertGradient(float t) {
        const float s = sqrt(clamp(1.0f - (t - 0.4f) / 0.6f, 0.0f, 1.0f));
        const Color3 sky = (Color3(1, 1, 1).lerp(Color3(0.0f, 0.8f, 1.0f), smoothstep(0.4f, 0.9f, t)) * Color3(s, s, 1.0f)).pow(0.5f);
        const Color3 land = Color3(0.7f, 0.3f, 0.0f).lerp(Color3(0.85f, 0.75f + max(0.8f - t * 20.0f, 0.0f), 0.5f), square(t / 0.4f));
        return ((t > 0.4f) ? sky : land).clamp(0.0f, 1.0f)* clamp(1.5f * (1.0f - fabs(t - 0.4f)), 0.0f, 1.0f);
    }


    static Color3 probeCoordVisualizationColor(Point3int32 P) {
        Color3 c(float(P.x & 1), float(P.y & 1), float(P.z & 1));
        // Make all probes the same brightness
        c /= max(c.r + c.g + c.b, 0.01f);
        return c * 0.6f + Color3(0.2f);
    }
#endif

    void DDGIVolume::loadGeometry
    (const String& filename,
        float                               scale,
        Color3                              faceColor,
        AttributeArray& vertexArray,
        IndexStream& indexStream) {

        ArticulatedModel::Specification spec;

        // Merge all geometry
        spec.filename = filename;
        spec.stripMaterials = true;
        spec.stripVertexColors = true;
        spec.scale = scale;
        spec.cleanGeometrySettings.allowVertexMerging = true;
        spec.cleanGeometrySettings.forceVertexMerging = true;
        spec.cleanGeometrySettings.maxNormalWeldAngle = finf();
        spec.cleanGeometrySettings.maxSmoothAngle = finf();
        spec.meshMergeOpaqueClusterRadius = finf();

        const shared_ptr<ArticulatedModel>& model = ArticulatedModel::create(spec);
        Array<shared_ptr<Surface>> surfaceArray;
        model->pose(surfaceArray, CFrame(), CFrame(), nullptr, nullptr, nullptr, Surface::ExpressiveLightScatteringProperties());

        Array<Tri> triArray;
        CPUVertexArray cvertexArray;
        Surface::getTris(surfaceArray, cvertexArray, triArray);

        Array<Vector3> position;
        position.resize(cvertexArray.size());
        for (int v = 0; v < cvertexArray.size(); ++v) {
            position[v] = cvertexArray.vertex[v].position;
        }

        Array<int> index;
        index.resize(triArray.size() * 3);
        for (int t = 0; t < triArray.size(); ++t) {
            const Tri& tri = triArray[t];
            for (int v = 0; v < 3; ++v) {
                index[t * 3 + v] = tri.getIndex(v);
            }
        }

        const shared_ptr<VertexBuffer>& area = VertexBuffer::create(position.size() * sizeof(Vector3) + index.size() * sizeof(int), VertexBuffer::WRITE_ONCE);
        vertexArray = AttributeArray(position, area);
        indexStream = IndexStream(index, area);
    }


    void DDGIVolume::debugRenderProbeVisualization(RenderDevice* rd,
        const shared_ptr<Camera>& camera,
        const bool visualizeDepth,
        const float probeVisualizationRadius) {

        rd->push2D(); {
            Args args;

            // Could recompute this only when the probe grid changes.
            args.setUniform("radius", probeVisualizationRadius * m_probeSpacing.length());
            // One instance for every probe.
            args.setNumInstances(probeCount());
            args.setPrimitiveType(PrimitiveType::TRIANGLES);

            args.setUniform("volumeColor", s_debugProbeVisualizationColors[m_debugProbeVisualizationColorsIndex]);
            args.setMacro("VISUALIZE_DEPTH", visualizeDepth);
            args.setMacro("VISUALIZE_VOLUME_COLOR", s_visualizeDebugColors);
            args.setUniform("maxDistance", m_maxDistance);
            args.setUniform("cameraWSPosition", camera->frame().translation);
            args.setAttributeArray("g3d_Vertex", s_debugProbeVisualizationVertexArray);
            args.setIndexStream(s_debugProbeVisualizationIndexStream);
            args.setUniform("probeSleeping", (int)m_probeSleeping);

            rd->setProjectionAndCameraMatrix(camera->projection(), camera->frame());
            rd->setObjectToWorldMatrix(CFrame());

            setShaderArgs(args, "ddgiVolume.");

            rd->setBlendFunc(RenderDevice::BLEND_ONE, RenderDevice::BLEND_ZERO);
            rd->setCullFace(CullFace::BACK);
            rd->setDepthWrite(true);
            rd->setDepthTest(RenderDevice::DEPTH_LEQUAL);

            LAUNCH_SHADER("DDGIVolume_probeVisualization.*", args);
        } rd->pop2D();
    }


    void DDGIVolume::debugDrawProbeLabels(float probeVisualizationRadius) const {
        for (int i = 0; i < probeCount(); ++i) {
            const Point3& probeCenter = probeIndexToPosition(i);
            debugDrawLabel(probeCenter,
                Vector3(0, 0, probeVisualizationRadius * m_probeSpacing.length() * 1.02f),
                GuiText(format("%d", i), s_guiLabelFont, -1.0f, Color4(Color3::fromASRGB(0xff007e))),
                0.0f,
                probeVisualizationRadius * m_probeSpacing.length() * 0.7f);
        }
    }


    void DDGIVolume::setShaderArgs(UniformTable& args, const String& prefix) {
        alwaysAssertM(endsWith(prefix, "."), "Requires a struct prefix");

        // Macros
        args.setMacro("FIRST_FRAME", m_firstFrame);

        const Sampler& bilinear = Sampler::video();

        // Irradiance
        args.setUniform(prefix + "irradianceTexture", m_irradianceTexture, bilinear);
        args.setUniform(prefix + "invIrradianceTextureSize", Vector2(1.0f, 1.0f) / m_irradianceTexture->vector2Bounds());
        args.setUniform(prefix + "irradianceProbeSideLength", irradianceOctSideLength());

        // Visibility
        args.setUniform(prefix + "visibilityTexture", m_visibilityTexture, bilinear);
        args.setUniform(prefix + "invVisibilityTextureSize", Vector2(1.0f, 1.0f) / m_visibilityTexture->vector2Bounds());
        args.setUniform(prefix + "visibilityProbeSideLength", visibilityOctSideLength());

        // Probe Offset (bound for reading and writing).
        args.setUniform(prefix + "probeOffsetsTexture", m_probeOffsetTexture, Sampler::buffer());
        args.setImageUniform(prefix + "probeOffsetsImage", m_probeOffsetTexture, Access::WRITE);
        args.setUniform(prefix + "probeOffsetLimit", m_specification.probeOffsetLimit);
        args.setMacro("OFFSET_BITS_PER_CHANNEL", m_probeOffsetTexture->format()->redBits);

        // Probe Grid
        args.setUniform(prefix + "probeCounts", m_specification.probeCounts);
        args.setUniform(prefix + "logProbeCounts", Vector3int32((int)::log(m_specification.probeCounts.x), (int)::log(m_specification.probeCounts.y), (int)::log(m_specification.probeCounts.z)));
        args.setUniform(prefix + "probeGridOrigin", m_probeGridOrigin);
        args.setUniform(prefix + "probeSpacing", m_probeSpacing);
        args.setUniform(prefix + "invProbeSpacing", Vector3::one() / m_probeSpacing);
        args.setUniform(prefix + "phaseOffsets", m_phaseOffset);

        // Scale the normal bias s.t. 1 is half the smallest probestep. This avoids overly large
        // (or small) bias for scenes without an explicit specification file.
        args.setUniform(prefix + "selfShadowBias", m_specification.selfShadowBias * 0.75f * m_probeSpacing.min());

        // Gamma encoding
        args.setUniform(prefix + "irradianceGamma", m_specification.irradianceGamma);
        args.setUniform(prefix + "invIrradianceGamma", 1.0f / m_specification.irradianceGamma);

        // Camera locked volume
        args.setUniform(prefix + "cameraLocked", m_specification.cameraLocked);
    }


    Point3int32 DDGIVolume::probeIndexToGridIndex(int index) const {
        const int xIndex = index % m_specification.probeCounts.x;
        const int yIndex = (index % (m_specification.probeCounts.x * m_specification.probeCounts.y)) / m_specification.probeCounts.x;
        const int zIndex = index / (m_specification.probeCounts.x * m_specification.probeCounts.y);
        return Point3int32(xIndex, yIndex, zIndex);
    }


    Point3 DDGIVolume::probeIndexToPosition(int index) const {
        const Point3int32 P = probeIndexToGridIndex(index);
        return m_probeSpacing * Vector3(P) + m_probeGridOrigin;
    }

    void DDGIVolume::updateAllProbeTypes
    (RenderDevice* rd,
        const Array<shared_ptr<Surface>>& surfaceArray,
        const shared_ptr<Texture>& rayHitLocations,
        const shared_ptr<Texture>& rayHitRadiance,
        const int									offset,
        const int                                   raysPerProbe) {

        BEGIN_PROFILER_EVENT("updateAllProbeTypes");

        m_lowIrradianceHysteresisFrames = max(0, m_lowIrradianceHysteresisFrames - 1);
        m_reducedIrradianceHysteresisFrames = max(0, m_reducedIrradianceHysteresisFrames - 1);

        // The compute shader will fail spectacularly unless these match.
        assert(irradianceOctSideLength() == 8 && visibilityOctSideLength() == 16);
        assert(raysPerProbe % 32 == 0);

        {

            m_rayBlockIndexOffset->bindAsShaderStorageBuffer(2);

            Args args;
            args.setUniform("randomOrientation", m_randomOrientation);

            float irradianceHysteresis = m_specification.hysteresis;
            float visibilityHysteresis = m_specification.hysteresis;

            if (m_firstFrame) {
                irradianceHysteresis = 0.0f;
                visibilityHysteresis = 0.0f;
            }
            else if (m_lowIrradianceHysteresisFrames > 0) {
                irradianceHysteresis *= 0.5f;
            }
            else if (m_reducedIrradianceHysteresisFrames > 0) {
                irradianceHysteresis *= 0.85f;
            }

            if (m_lowVisibilityHysteresisFrames > 0) {
                visibilityHysteresis *= 0.5f;
            }

            args.setImageUniform("irradianceImage", m_irradianceTexture, Access::WRITE);
            args.setImageUniform("visibilityImage", m_visibilityTexture, Access::WRITE);

            args.setMacro("RAYS_PER_PROBE", raysPerProbe);
            args.setUniform("irradianceHysteresis", irradianceHysteresis);
            args.setUniform("visibilityHysteresis", visibilityHysteresis);
            args.setUniform("visibilitySharpness", m_specification.depthSharpness);

            args.setUniform("maxDistance", m_maxDistance);
            setShaderArgs(args, "ddgiVolume.");

            args.setUniform("probeSleeping", (int)m_probeSleeping);

            args.setUniform("offset", offset);

            rayHitLocations->setShaderArgs(args, "rayHitLocations.", Sampler::buffer());
            rayHitRadiance->setShaderArgs(args, "rayHitRadiance.", Sampler::buffer());

            const Vector3int32& p = probeCounts();
            // Don't change this, 
            const int computeThreadsPerTexel = 1;
            // (8*8 + 16*16) / 32 = 20;

            const int convolutionBlockSize = (raysPerProbe % 64 == 0) ? 64 : 32;
            debugAssertM(raysPerProbe % 32 == 0, "raysPerProbe must be at least a multiple of 32");

            const int gridZ = (square(irradianceOctSideLength()) + square(visibilityOctSideLength())) * computeThreadsPerTexel / convolutionBlockSize;
            args.setComputeGridDim(Vector3int32(p.x * p.y, p.z, gridZ));
            args.setComputeGroupSize(Vector3int32(convolutionBlockSize, 1, 1));
            args.setMacro("BLOCK_SIZE", convolutionBlockSize);

            debugAssertGLOk();
            LAUNCH_SHADER("DDGIVolume_updateProbes.glc", args);
        }

        debugAssertGLOk();

        // Compute shader launch
        {
            Args args;

            args.setImageUniform("irradianceImage", m_irradianceTexture, Access::WRITE);
            args.setImageUniform("visibilityImage", m_visibilityTexture, Access::WRITE);

            args.setUniform("irradianceSide", irradianceOctSideLength());
            args.setUniform("visibilitySide", visibilityOctSideLength());

            args.setUniform("irradianceTexture", m_irradianceTexture, Sampler::buffer());
            args.setUniform("visibilityTexture", m_visibilityTexture, Sampler::buffer());

            const Vector3int32& p = probeCounts();


            // Compute shader launch
            // blocksize = (8,4,1)
            // Launch 13 blocks per 4 probes (0 = corners, 1-4 = irradiance, 5-12 = depth)
            const int blocksPer4Probes = 13;

            assert(p.x * p.y % 2 == 0 && p.z % 2 == 0);
            args.setComputeGridDim(Vector3int32(p.x * p.y / 2, p.z / 2, blocksPer4Probes));

            LAUNCH_SHADER("DDGIVolume_copyProbeEdges.glc", args);
        }
        END_PROFILER_EVENT();
    }


    void DDGIVolume::resizeIfNeeded() {
        BEGIN_PROFILER_EVENT("resizeIfNeeded()");
        const int irradianceSide = irradianceOctSideLength();
        const int visibilitySide = visibilityOctSideLength();

        static int oldIrradianceSide = 0;
        static int oldVisibilitySide = 0;

        // Allocate irradiance/depth probes if this is the first call or the probe resolution changes (mostly for debugging; in normal use,
        // this is only invoked once anyway)
        if (isNull(m_irradianceTexture) ||
            (irradianceSide != oldIrradianceSide) ||
            (visibilitySide != oldVisibilitySide) ||
            (m_irradianceTexture->format() != s_guiIrradianceFormats[m_guiIrradianceFormatIndex]) ||
            (m_visibilityTexture->format() != s_guiDepthFormats[m_guiVisibilityFormatIndex]) ||
            m_guiProbeFormatChanged) {

            const int probeXY = m_specification.probeCounts.x * m_specification.probeCounts.y;
            const int probeZ = m_specification.probeCounts.z;

            const int irradianceWidth = (irradianceSide + 2) * probeXY;
            const int irradianceHeight = (irradianceSide + 2) * probeZ;

            const int visibilityWidth = (visibilitySide + 2) * probeXY;
            const int visibilityHeight = (visibilitySide + 2) * probeZ;

            m_irradianceTexture = Texture::createEmpty("DDGIVolume::m_irradianceTexture", irradianceWidth, irradianceHeight,
                s_guiIrradianceFormats[m_guiIrradianceFormatIndex], Texture::DIM_2D, false, 1);
            m_irradianceTexture->visualization.documentGamma = 5.0f;

            m_guiProbeFormatChanged = false;

            m_visibilityTexture = Texture::createEmpty("DDGIVolume::m_visibilityTexture", visibilityWidth, visibilityHeight,
                s_guiDepthFormats[m_guiVisibilityFormatIndex], Texture::DIM_2D, false, 1);

            m_visibilityTexture->visualization.channels = Texture::Visualization::RasL;
            m_visibilityTexture->visualization.max = 30.0f;

            // If we're creating this texture, can't have useful data; all probes should be uninitialized.
            m_probeOffsetTexture = Texture::createEmpty("DDGIVolume::m_probeOffsetTexture", probeXY, probeZ, ImageFormat::RGBA8I());
            m_rayBlockIndexOffset = GLPixelTransferBuffer::create(256, iCeil((float)probeCount() / 256.0f), ImageFormat::R32I());
            setProbeStatesToUninitialized();
        }

        oldIrradianceSide = irradianceSide;
        oldVisibilitySide = visibilitySide;
        END_PROFILER_EVENT();
    }

    Vector4int8* DDGIVolume::mapSleepingProbesBuffer(bool forWriting) {
        if (isNull(m_sleepingProbesBuffer)) {
            m_sleepingProbesBuffer =
                GLPixelTransferBuffer::create(m_probeOffsetTexture->width(),
                    m_probeOffsetTexture->height(),
                    m_probeOffsetTexture->format());
        }

        BEGIN_PROFILER_EVENT("Grow Buffer");
        // Only grow the buffer
        if (m_sleepingProbesBuffer->width() < m_probeOffsetTexture->width() || m_sleepingProbesBuffer->height() < m_probeOffsetTexture->height()) {
            m_sleepingProbesBuffer->resize(m_probeOffsetTexture->width(), m_probeOffsetTexture->height());
        }
        END_PROFILER_EVENT();

        m_probeOffsetTexture->toPixelTransferBuffer(m_sleepingProbesBuffer);

        return (Vector4int8*)(forWriting ? m_sleepingProbesBuffer->mapWrite() : m_sleepingProbesBuffer->mapRead());
    }

    void DDGIVolume::unmapSleepingProbesBuffer() {
        m_sleepingProbesBuffer->unmap();
    }

    void DDGIVolume::gatherTracingProbes(const Array<ProbeStates>& states) {
        resizeIfNeeded();

        Vector4int8* sleepingProbesData = mapSleepingProbesBuffer();

        BEGIN_PROFILER_EVENT("Copy sleeping data to CPU");
        int* rayIndexOffsets = (int*)m_rayBlockIndexOffset->mapWrite();

        m_skippedProbes = 0;
        for (int i = 0; i < probeCount(); ++i) {
            const ProbeStates& state = ProbeStates(sleepingProbesData[i].w);

            // Only generate rays for probes in the state that we are tracing for.
            if (!states.contains(state) && states.size() > 0 && m_probeSleeping) {
                ++m_skippedProbes;
                rayIndexOffsets[i] = -1;
            }
            else {
                rayIndexOffsets[i] = m_skippedProbes;
            }
        }

        unmapSleepingProbesBuffer();
        sleepingProbesData = nullptr;

        m_rayBlockIndexOffset->unmap();
        rayIndexOffsets = nullptr;
        END_PROFILER_EVENT();
    }

    void DDGIVolume::notifyOfDynamicObjects(const Array<AABox>& currentBoxArray, const Array<Vector3>& velocityArray) {

        // Even if this is 0, we can't early exit because we still need
        // the shader to run to update the flags.
        const int totalBoundingBoxes = currentBoxArray.size();

        // The buffer will be accessed 1D in the shader. It's 2D here solely to make clear that
        // we have a hi and lo point for each bounding box.
        if (isNull(m_conservativeAABoundsPBO)) {
            // If totalBoundingBoxes is 0, this will quietly fail in the shader, but we need
            // the creation to work here.
            m_conservativeAABoundsPBO = GLPixelTransferBuffer::create(max(totalBoundingBoxes, 1), 2, ImageFormat::RGBA32F());
        }
        else if (m_conservativeAABoundsPBO->width() < totalBoundingBoxes) {
            m_conservativeAABoundsPBO->resize(totalBoundingBoxes, 2);
        }

        Vector4* conservativeAABoundsData = (Vector4*)m_conservativeAABoundsPBO->mapWrite();

        // Copy the data to the GPU.
        runConcurrently(Point2int32(0, 0), Point2int32(totalBoundingBoxes, 2), [&](Point2int32 index) {
            const AABox& aabox = currentBoxArray[index.x];

            Point3 bound = (index.y == 0) ? aabox.low() : aabox.high();

            // Extrude the AABB along the probe axes
            // Needs to expand a full probe step in each direction.
            bound += (2 * index.y - 1) * m_probeSpacing;

            conservativeAABoundsData[2 * index.x + index.y] = Vector4(bound, 1);
            });

        m_conservativeAABoundsPBO->unmap();
        conservativeAABoundsData = nullptr;

        {
            Args args;

            m_conservativeAABoundsPBO->bindAsShaderStorageBuffer(2);

            debugAssert(notNull(m_probeOffsetTexture));
            args.setRect(m_probeOffsetTexture->rect2DBounds());
            setShaderArgs(args, "ddgiVolume.");

            args.setUniform("totalBoundingBoxes", totalBoundingBoxes);

            const Vector3int32& p = probeCounts();

            // Assume we never go below 4x2x4 probes
            assert(p.x * p.y % 8 == 0);
            assert(p.z % 4 == 0);

            // Using 8x4 threadblocks for perfect coherency over probe dimensions 
            // at 32 threads/block.
            const Vector2int32 blockDims(8, 4);

            args.setComputeGridDim(Vector3int32(p.x * p.y / blockDims.x, p.z / blockDims.y, 1));
            args.setComputeGroupSize(Vector3int32(blockDims, 1));

            LAUNCH_SHADER("DDGIVolume_updateProbeFlags.glc", args);
        }

    }

    bool DDGIVolume::notifyOfCameraPosition(const Point3& cameraWSPosition) {
        if (!cameraLocked()) {
            return false;
        }

        // Centerpoint of the volume. For even axes, this will be in the center of the central 8 probe cage.
        const Point3& volumeCenter = m_probeGridOrigin + m_probeSpacing * (Vector3(probeCounts() - Vector3int32(1, 1, 1)) / 2.0f);
        const AABox& volumeCenterRegion = AABox(volumeCenter - m_probeSpacing, volumeCenter + m_probeSpacing);

        // If the camera is within our center volume, don't need to move.
        if (volumeCenterRegion.contains(cameraWSPosition)) {
            return false;
        }

        // Move 2 probespacing lengths along the axis that would get the camera closest to the new centerpoint

        // Distance to center, pointing towards the camera to make computation easier.
        const Vector3& vectorToCenter = cameraWSPosition - volumeCenter;

        const Array<Vector3int32> movementOptions = { Vector3int32(1, 0, 0),
                                                     Vector3int32(-1, 0, 0),
                                                     Vector3int32(0, 1, 0),
                                                     Vector3int32(0, -1, 0),
                                                     Vector3int32(0, 0, 1),
                                                     Vector3int32(0, 0, -1) };

        int index = -1;
        float length = vectorToCenter.length();
        for (int i = 0; i < movementOptions.size(); ++i) {
            float newLength = (vectorToCenter - Vector3(movementOptions[i]) * m_probeSpacing).length();
            if (newLength < length) {
                length = newLength;
                index = i;
            }
        }

        m_probeGridOrigin += Vector3(movementOptions[index]) * m_probeSpacing;

        // Phase offset works opposite the motion of the camera.
        m_phaseOffset -= movementOptions[index];

        m_uninitializedPlane = m_phaseOffset * Vector3int32(abs(movementOptions[index]));

        Vector4int8* sleepingProbesData = mapSleepingProbesBuffer(true);


        for (int i = 0; i < probeCount(); ++i) {
            Vector3int32 iPos;
            iPos.x = i % probeCounts().x;
            iPos.y = (i % (probeCounts().x * probeCounts().y)) / probeCounts().x;
            iPos.z = i / (probeCounts().x * probeCounts().y);

            bool uninit = false;
            for (int j = 0; j < 3; ++j) {
                if (movementOptions[index][j] == 0) {
                    continue;
                }
                iPos[j] = (iPos[j] + m_phaseOffset[j]) % probeCounts()[j];
                if (iPos[j] < 0) {
                    iPos[j] = probeCounts()[j] + iPos[j];
                }
                if (iPos[j] == max(0, movementOptions[index][j] * (probeCounts()[j] - 1))) {
                    uninit = true;
                }
            }
            if (uninit) {
                sleepingProbesData[i] = Vector4int8(0, 0, 0, DDGIVolume::ProbeStates::UNINITIALIZED);
            }
        }

        unmapSleepingProbesBuffer();
        sleepingProbesData = nullptr;

        m_probeOffsetTexture->update(m_sleepingProbesBuffer);

        return true;
    }
}

