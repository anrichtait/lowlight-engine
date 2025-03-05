/**
  \file G3D-app.lib/source/UniversalMaterial.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-app/UniversalMaterial.h"
#include "G3D-base/Table.h"
#include "G3D-base/WeakCache.h"
#include "G3D-base/FileSystem.h"
#include "G3D-app/UniversalSurfel.h"
#include "G3D-gfx/Args.h"
#include "G3D-gfx/Sampler.h"

#ifdef OPTIONAL
#   undef OPTIONAL
#endif

namespace G3D {    

void UniversalMaterial::sample(const Tri& tri, float u, float v, int triIndex, const CPUVertexArray& vertexArray, bool backside, shared_ptr<Surfel>& surfel, float du, float dv, bool twoSided) const {
    // Avoid the atomic increment cost of a dynamic_ptr_cast, since we know the object can't be colleted during this call
    UniversalSurfel* universalSurfel = dynamic_cast<UniversalSurfel*>(surfel.get());
    if (universalSurfel) {
        // Reuse the existing surfel
        universalSurfel->sample(tri, u, v, triIndex, vertexArray, backside, this, du, dv, twoSided);
    } else {
        surfel = std::make_shared<UniversalSurfel>(tri, u, v, triIndex, vertexArray, backside, du, dv);
    }
    surfel->flags = m_flags;
}


UniversalMaterial::UniversalMaterial() :
    m_numLightMapDirections(0),
    m_inferAmbientOcclusionAtTransparentPixels(true) {
}


shared_ptr<UniversalMaterial> UniversalMaterial::createEmpty() {
    return createShared<UniversalMaterial>();
}


shared_ptr<UniversalMaterial> UniversalMaterial::create
(const shared_ptr<UniversalBSDF>&    bsdf,
 const Component3&                   emissive,
 const shared_ptr<BumpMap>&          bump,
 const Array<Component3>&            lightMaps,
 const String&                       customShaderPrefix,
 const AlphaFilter                   alphaFilter,
 const uint8                         flags) {

    alwaysAssertM((lightMaps.size() <= 1) || (lightMaps.size() == 3), 
        format("Tried to create material with %d directional lightMaps, must have 0, 1, or 3", lightMaps.size()));

    const shared_ptr<UniversalMaterial>& m = UniversalMaterial::createEmpty();

    if (notNull(bsdf)) {
        const shared_ptr<Texture>& texture = bsdf->lambertian().texture();
        if (texture) {
            m->m_name = texture->name();
        }
    }

    m->m_bsdf       = bsdf;
    m->m_emissive   = emissive;
    m->m_bump       = bump;
    m->m_flags      = flags;

    if (alphaFilter == AlphaFilter::DETECT) {
        if (bsdf->lambertian().texture()->alphaFilter() == AlphaFilter::DETECT) {
            if (bsdf->lambertian().max().a < 1.0f) {
                m->m_alphaFilter = AlphaFilter::BLEND;
            } else {
                m->m_alphaFilter = AlphaFilter::ONE;
            }
        } else {
            m->m_alphaFilter = bsdf->lambertian().texture()->alphaFilter();
        }
    } else {
        m->m_alphaFilter = alphaFilter;
    }

    debugAssert(m->m_bsdf->lambertian().texture());
    debugAssert(m->m_bsdf->glossy().texture());

    m->m_numLightMapDirections = lightMaps.size();
    for (int i = 0; i < lightMaps.size(); ++i) {
        m->m_lightMap[i] = lightMaps[i];
    }

    m->m_customShaderPrefix = customShaderPrefix;

    m->computeDefines(m->m_macros);

    return m;
}


shared_ptr<UniversalMaterial> UniversalMaterial::createDiffuse(const Color3& p_Lambertian) {
    Specification s;
    s.setLambertian(Texture::Specification(p_Lambertian));

    return create(s);
}


shared_ptr<UniversalMaterial> UniversalMaterial::createDiffuse(const String& lambertianFilename) {
    Specification s;
    s.setLambertian(Texture::Specification(lambertianFilename, true));

    return create(s);
}


shared_ptr<UniversalMaterial> UniversalMaterial::createDiffuse(const shared_ptr<Texture>& texture){
    Specification s;
    s.setLambertian(texture);

    return create(s);
}


typedef WeakCache<UniversalMaterial::Specification, shared_ptr<UniversalMaterial> > MaterialCache;

namespace _internal {
    WeakCache<UniversalMaterial::Specification, shared_ptr<UniversalMaterial> >& materialCache() {
        static WeakCache<UniversalMaterial::Specification, shared_ptr<UniversalMaterial> > c;
        return c;
    }
}


void UniversalMaterial::clearCache() {
    _internal::materialCache().clear();
}


shared_ptr<UniversalMaterial> UniversalMaterial::create(const Specification& specification) {
    return create(FilePath::base(specification.m_lambertian.filename), specification);
}

bool UniversalMaterial::validateTextureDimensions() const {
    // Check all textures in the material.
    SmallArray<shared_ptr<Texture>, 5> textures;
    textures.append(m_bsdf->lambertian().texture());
    textures.append(m_bsdf->glossy().texture());
    textures.append(m_bsdf->transmissive().texture());

    if (notNull(m_emissive.texture())) {
        textures.append(m_emissive.texture());
    }

    if (notNull(m_bump) && notNull(m_bump->normalBumpMap())) {
        textures.append(m_bump->normalBumpMap()->texture());
    }
    Texture::Dimension dim = textures[0]->dimension();
    bool allSame = true;
    for (int i = 0; i < textures.size(); ++i) {
        allSame = allSame && (textures[i]->dimension() == dim);
    }
    return allSame;
}


shared_ptr<UniversalMaterial> UniversalMaterial::create(const String& name, const Specification& specification) {
    MaterialCache& cache = _internal::materialCache();
    shared_ptr<UniversalMaterial> value = cache[specification];

    if (isNull(value)) {
        // Construct the appropriate material
        value = UniversalMaterial::createEmpty();

        value->m_bsdf =
            UniversalBSDF::create(
                specification.loadLambertian(),
                specification.loadGlossy(),
                specification.loadTransmissive(),
                specification.m_etaTransmit,
                specification.m_extinctionTransmit,
                specification.m_etaReflect,
                specification.m_extinctionReflect);

        debugAssert(value->m_bsdf->lambertian().texture());
        debugAssert(value->m_bsdf->glossy().texture());

        value->m_customShaderPrefix     = specification.m_customShaderPrefix;
        value->m_refractionHint         = specification.m_refractionHint;
        value->m_mirrorHint             = specification.m_mirrorHint;
        value->m_alphaFilter            = specification.m_alphaFilter;
        value->m_sampler                = specification.m_sampler;

        value->m_flags                  = specification.m_flags;

        // load emission map
        value->m_emissive = specification.loadEmissive();

        // load bump map
        if (! specification.m_bump.texture.filename.empty()) {
            value->m_bump = BumpMap::create(specification.m_bump);
        }

        value->m_name = name;

        value->m_constantTable = specification.m_constantTable;

        value->m_numLightMapDirections = specification.m_numLightMapDirections;
        for (int i = 0; i < specification.m_numLightMapDirections; ++i) {
            value->m_lightMap[i] = Component3(specification.m_lightMap[i]);
        }

        // Update the cache
        cache.set(specification, value);

        if (specification.alphaFilter() == AlphaFilter::DETECT) {
            const shared_ptr<Texture>& lambertian = value->bsdf()->lambertian().texture();
            if (lambertian->alphaFilter() == AlphaFilter::DETECT) {
                if (lambertian->conservativelyHasUnitAlpha()) {
                    value->m_alphaFilter = AlphaFilter::ONE;
                } else if (lambertian->conservativelyHasNonUnitAlpha() || (lambertian->max().a < 1.0f)) {
                    value->m_alphaFilter = AlphaFilter::BLEND;
                } else {
                    value->m_alphaFilter = AlphaFilter::ONE;
                }
            } else {
                value->m_alphaFilter = lambertian->alphaFilter();
            }
        } else {
            value->m_alphaFilter = specification.alphaFilter();
        }

        if (specification.inferAmbientOcclusionAtTransparentPixels().type() == Any::BOOLEAN) {
            value->m_inferAmbientOcclusionAtTransparentPixels = specification.inferAmbientOcclusionAtTransparentPixels().boolean();
        } else { 
            // Detect partial coverage
            specification.inferAmbientOcclusionAtTransparentPixels().verifyType(Any::STRING);
            value->m_inferAmbientOcclusionAtTransparentPixels = (! value->hasTransmissive()) && value->hasAlpha() && (value->bsdf()->lambertian().max().a == 1.0);
        }
    
        value->m_needsComputeDefines = true;
        debugAssert(notNull(value->bsdf()->lambertian().texture()));
        debugAssertM(value->validateTextureDimensions(), "Not all texture in material are the same dimension!");
    }
    return value;
}


void UniversalMaterial::setStorage(ImageStorage s) const {
    m_bsdf->setStorage(s);
    
    m_emissive.setStorage(s);
    
    if (m_bump) {
        m_bump->setStorage(s);
    }
}


bool UniversalMaterial::hasTransmissive() const {
    return notNull(m_bsdf->transmissive().texture()) && (m_bsdf->transmissive().texture()->max().rgb().max() > 0);
}


bool UniversalMaterial::hasEmissive() const {
    return notNull(m_emissive.texture()) && (m_emissive.texture()->max().rgb().max() > 0);
}


void UniversalMaterial::setShaderArgs(UniformTable& args, const String& prefix) const {
    const bool structStyle = prefix.find('.') != std::string::npos;

    // Invoke the accessor in case of lazy loading
    args.appendToPreamble(macros());
    static const bool OPTIONAL = true;

    args.setMacro("NUM_LIGHTMAP_DIRECTIONS", m_numLightMapDirections);

    // Construct the MIP-mapless version of this sampler
    Sampler noMipSampler(m_sampler);
    switch (m_sampler.interpolateMode) {
    case InterpolateMode::NEAREST_MIPMAP:
        noMipSampler.interpolateMode = InterpolateMode::NEAREST_NO_MIPMAP;
        break;

    default:
        noMipSampler.interpolateMode = InterpolateMode::BILINEAR_NO_MIPMAP;
        break;
    }

    Texture::Dimension dim = textureDimension();
    const shared_ptr<Texture>& textureZero          = Texture::zero(dim);
    const shared_ptr<Texture>& textureOpaqueBlack   = Texture::opaqueBlack(dim);
    debugAssert(m_bsdf->glossy().texture());
    m_bsdf->lambertian().texture()->setShaderArgs(args, prefix + (structStyle ? "lambertian." : "LAMBERTIAN_"), 
        m_bsdf->lambertian().texture()->hasMipMaps() ? m_sampler : noMipSampler);

    if (! structStyle) {
        args.setMacro(prefix + "HAS_ALPHA", (m_alphaFilter != AlphaFilter::ONE) && ! m_bsdf->lambertian().texture()->opaque());
    }

    args.setUniform(prefix + "flags", (unsigned int)(m_flags));

    m_bsdf->glossy().texture()->setShaderArgs(args, prefix + (structStyle ? "glossy." : "GLOSSY_"), 
        m_bsdf->glossy().texture()->hasMipMaps() ? m_sampler : noMipSampler);

    if (hasEmissive()) {
        m_emissive.texture()->setShaderArgs(args, prefix + (structStyle ? "emissive." : "EMISSIVE_"), 
            m_emissive.texture()->hasMipMaps() ? m_sampler : noMipSampler);
    } else if (structStyle) {
        textureOpaqueBlack->setShaderArgs(args, prefix + "emissive.", noMipSampler);
    }
    
    if (hasTransmissive()) {
        m_bsdf->transmissive().texture()->setShaderArgs
            (args, prefix + (structStyle ? "transmissive." : "TRANSMISSIVE_"),
             m_bsdf->transmissive().texture()->hasMipMaps() ? m_sampler : noMipSampler);
    } else if (structStyle) {
        textureOpaqueBlack->setShaderArgs(args, prefix + "transmissive.", noMipSampler);
    }
    args.setUniform(prefix + "etaTransmit", m_bsdf->etaTransmit(), OPTIONAL);
    args.setUniform(prefix + "etaRatio", m_bsdf->etaReflect() / m_bsdf->etaTransmit(), OPTIONAL);


    if (m_bump && (m_bump->settings().scale != 0)) {
        const shared_ptr<Texture>& tex = m_bump->normalBumpMap()->texture();
        alwaysAssertM(tex->hasMipMaps(), format("No MIP-maps on texture '%s'.", tex->name().c_str()));
        args.setUniform(prefix + "normalBumpMap",               tex, tex->hasMipMaps() ? m_sampler : noMipSampler, OPTIONAL);
        if (m_bump->settings().iterations > 0) {
            args.setUniform(prefix + "bumpMapScale",            m_bump->settings().scale, OPTIONAL);
            args.setUniform(prefix + "bumpMapBias",             m_bump->settings().bias, OPTIONAL);
        }
    } else if (structStyle) {
        args.setUniform(prefix + "normalBumpMap", textureZero, noMipSampler, OPTIONAL);
        args.setUniform(prefix + "bumpMapScale", 1.0f, OPTIONAL);
        args.setUniform(prefix + "bumpMapBias",  0.0f, OPTIONAL);
    }

#   ifndef G3D_OSX
    if (m_numLightMapDirections > 0) {
        m_lightMap[0].texture()->setShaderArgs(args, prefix + (structStyle ? "lightMap0." : "lightMap0_"), Sampler::lightMap());
                
        if (m_numLightMapDirections == 3) { 
            alwaysAssertM((m_lightMap[0].texture()->encoding().readMultiplyFirst == m_lightMap[1].texture()->encoding().readMultiplyFirst) &&
                          (m_lightMap[0].texture()->encoding().readMultiplyFirst == m_lightMap[2].texture()->encoding().readMultiplyFirst) &&
                          (m_lightMap[0].texture()->encoding().readAddSecond == Color4::zero()) &&
                          (m_lightMap[1].texture()->encoding().readAddSecond == Color4::zero()) &&
                          (m_lightMap[2].texture()->encoding().readAddSecond == Color4::zero()), 
                          "Lightmaps must have identical readMultiplyFirsts and zero readAddSeconds");
            
            m_lightMap[1].texture()->setShaderArgs(args, prefix + (structStyle ? "lightMap1." : "lightMap1_"), Sampler::lightMap());
            m_lightMap[2].texture()->setShaderArgs(args, prefix + (structStyle ? "lightMap2." : "lightMap2_"), Sampler::lightMap());
        } else if (structStyle) {
            textureOpaqueBlack->setShaderArgs(args, prefix + "lightMap1.", Sampler::lightMap());
            textureOpaqueBlack->setShaderArgs(args, prefix + "lightMap2.", Sampler::lightMap());
        }
    } else if (structStyle) {
        textureOpaqueBlack->setShaderArgs(args, prefix + "lightMap0.", Sampler::lightMap());
        textureOpaqueBlack->setShaderArgs(args, prefix + "lightMap1.", Sampler::lightMap());
        textureOpaqueBlack->setShaderArgs(args, prefix + "lightMap2.", Sampler::lightMap());
    }
#   else
    static bool alreadyPrinted = false;
    if (! alreadyPrinted && m_numLightMapDirections > 0) {
        debugPrintf("Light maps are not supported on OS X because it does not support a recent version of OpenGL and runs out of texture units.\n");
        alreadyPrinted = true;
    }
#   endif
    
    if (structStyle) {
        args.setUniform(prefix + "alphaFilter", m_alphaFilter.value);
    } else {
        args.setMacro(prefix + "alphaFilter", m_alphaFilter.value);
    }

    debugAssert(isNull(m_bump) || m_bump->settings().iterations >= 0);
}


void UniversalMaterial::computeDefines(String& defines) const {    
    if (m_bsdf->hasMirror()) {
        defines += "#define MIRROR\n";
    }

    if (notNull(m_bump) && (m_bump->settings().scale != 0)) {
        defines += "#define HAS_NORMAL_BUMP_MAP 1\n";
        defines += format("#define PARALLAXSTEPS (%d)\n", m_bump->settings().iterations);
    } else {
        defines += "#define HAS_NORMAL_BUMP_MAP 0\n";
        defines += "#define PARALLAXSTEPS 0\n";
    }

    for (Table<String, double>::Iterator it = m_constantTable.begin(); it.hasMore(); ++it) {
        defines += format("#define %s (%g)\n", it->key.c_str(), it->value);
    }

    defines += m_customShaderPrefix;
}


Texture::Dimension UniversalMaterial::textureDimension() const {
    debugAssertM(validateTextureDimensions(), "Not all texture in material are the same dimension!");
    return m_bsdf->lambertian().texture()->dimension();
}


bool UniversalMaterial::hasPartialCoverage() const {
    return bsdf()->lambertian().min().a < 1.0f;
}


bool UniversalMaterial::coverageLessThanEqual(const float alphaThreshold, const Point2& texCoord) const {
    const Component4& lambertian = bsdf()->lambertian();

    if (lambertian.min().a > alphaThreshold) {
        // Opaque pixel
        return false;
    }
            
    const shared_ptr<Image4>& image = lambertian.image();

    const Point2& t = texCoord * Vector2(float(image->width()), float(image->height()));

    return (image->nearest(t).a < alphaThreshold);
}


bool UniversalMaterial::hasAlpha() const {
    const shared_ptr<Texture>& lamb = m_bsdf->lambertian().texture();
    return (alphaFilter() != AlphaFilter::ONE) && 
        notNull(lamb) && 
        ! lamb->opaque() &&
        (lamb->min().a < 1.0f);
}


} // namespace G3D
