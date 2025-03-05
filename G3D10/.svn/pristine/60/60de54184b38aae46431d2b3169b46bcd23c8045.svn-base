/**
  \file G3D-app.lib/source/Light.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/Sphere.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/Any.h"
#include "G3D-base/stringutils.h"
#include "G3D-base/units.h"
#include "G3D-base/Random.h"
#include "G3D-app/Light.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/ShadowMap.h"
#include "G3D-base/Cone.h"
#include "G3D-gfx/GLCaps.h"
#include "G3D-app/GApp.h"
#include "G3D-app/GuiPane.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-app/ArticulatedModel.h"

namespace G3D {

shared_ptr<Entity> Light::create(const String& name, Scene* scene, AnyTableReader& propertyTable, const ModelTable& modelTable,
    const Scene::LoadOptions& options) {

    const shared_ptr<Light>& light = createShared<Light>();

    light->Entity::init(name, scene, propertyTable);
    light->VisibleEntity::init(propertyTable, modelTable);
    light->Light::init(name, propertyTable);
    propertyTable.verifyDone();

    return light;
}


void Light::init(const String& name, AnyTableReader& propertyTable){
    propertyTable.getIfPresent("extent", m_extent);
    propertyTable.getIfPresent("bulbPower", color);
    propertyTable.getIfPresent("bulbPowerTrack", m_bulbPowerTrack);
    propertyTable.getIfPresent("biradiance", color);
    propertyTable.getIfPresent("type", m_type);
    propertyTable.getIfPresent("varianceShadowSettings", m_varianceShadowSettings);

    Any temp;
    if (propertyTable.getIfPresent("shadowCullFace", temp)) {
        m_shadowCullFace = temp;
        temp.verify(m_shadowCullFace != CullFace::CURRENT, "A Light's shadowCullFace may not be CURRENT.");
    }

    // Default shadow maps to 2048 by 2048
    Vector2int16 shadowMapSize(2048, 2048);
    if (propertyTable.getIfPresent("shadowMapSize", shadowMapSize)) {
        // If the light has a shadowmap, it probably should cast shadows by default
        m_shadowsEnabled = true;
    }

    propertyTable.getIfPresent("shadowsEnabled", m_shadowsEnabled);

    propertyTable.getIfPresent("spotHardness", m_spotHardness);
    propertyTable.getIfPresent("producesIndirectIllumination", m_producesIndirectIllumination);
    propertyTable.getIfPresent("producesDirectIllumination", m_producesDirectIllumination);
    propertyTable.getIfPresent("areaLightPullback", m_areaLightPullback);

    propertyTable.getIfPresent("enabled",       m_enabled);
    
    float spotHalfAngleDegrees = 180.0f;
    propertyTable.getIfPresent("spotHalfAngleDegrees", spotHalfAngleDegrees);
    m_spotHalfAngle = toRadians(spotHalfAngleDegrees);

    propertyTable.getIfPresent("rectangular",    m_rectangular);

    Any test;
    if (propertyTable.getIfPresent("nearPlaneZLimit", test)) {
        m_nearPlaneZLimit = test;
        test.verify(m_nearPlaneZLimit < 0, "nearPlaneZLimit must be negative");
    }

    if (propertyTable.getIfPresent("farPlaneZLimit", test)) {
        m_farPlaneZLimit = test;
        test.verify(m_farPlaneZLimit < m_nearPlaneZLimit, "farPlaneZLimit must be less than nearPlaneZLimit");
    }    
    
    const float radius = m_extent.length() / 2.0f;
    Array<float> tempAtt = { square(radius), 0.0f, 1.0f };
    if (m_type == Type::DIRECTIONAL) {
        // No attenuation on directional lights by default
        tempAtt[0] = 1.0f;
        tempAtt[1] = 0.0f;
        tempAtt[2] = 0.0f;
    }

    propertyTable.getIfPresent("attenuation",  tempAtt);
    for (int i = 0; i < 3; ++i) {
        attenuation[i] = tempAtt[i];
    }

    float f;
    bool hasShadowMapBias = propertyTable.getIfPresent("shadowMapBias", f);
    if (m_shadowsEnabled && (min(shadowMapSize.x, shadowMapSize.y) > 0)) {
        m_shadowMap = ShadowMap::create("G3D::ShadowMap for " + name, shadowMapSize, m_varianceShadowSettings);
        if (hasShadowMapBias) {
            m_shadowMap->setBias(f);
        }

        propertyTable.any().verify(m_type != Type::OMNI, "Cannot have a shadow map on an OMNI light");
    }
}


int Light::findBrightestLightIndex(const Array<shared_ptr<Light> >& array, const Point3& point) {
    Radiance r = -finf();
    int index = -1;
    for (int i = 0; i < array.length(); ++i) {
        const Radiance t = array[i]->biradiance(point).sum();
        if (t > r) {
            r = t;
            index = i;
        }
    }

    return index;
}


shared_ptr<Light> Light::findBrightestLight(const Array<shared_ptr<Light> >& array, const Point3& point) {
    if (array.size() > 0) {
        return array[findBrightestLightIndex(array, point)];
    } else {
        return nullptr;
    }
}


Power3 Light::bulbPower() const {
    if (m_type == Type::DIRECTIONAL) {
        // Directional lights have infinite power
        return Power3((color.r > 0) ? color.r * finf() : 0, (color.g > 0) ? color.g * finf() : 0, (color.b > 0) ? color.b * finf() : 0);
    } else {
        return color;
    }
}


Power3 Light::emittedPower() const {
    switch (m_type) {
    case Type::OMNI:
    case Type::DIRECTIONAL:
        return bulbPower();

    case Type::AREA:
    default:
        // Bulb power scaled by the fraction of sphere's solid angle subtended
        return bulbPower() / (4.0f * pif());

    case Type::SPOT:
        // Bulb power scaled by the fraction of sphere's solid angle subtended.
        return (1.0f - cos(spotHalfAngle())) * bulbPower() * 0.5f *
            (rectangular() ? (4.0f / pif()) : 1.0f); // area of rectangle vs. disk
    }
}


bool Light::inFieldOfView(const Vector3& wi) const {
    const CFrame& lightFrame = frame();

    switch (type()) {
    case Type::OMNI:
        return true;

    case Type::DIRECTIONAL:
        return wi == -lightFrame.lookVector();

    case Type::SPOT:
        {
            const float threshold = -cos(spotHalfAngle());
            if (rectangular()) {
                // Project wi onto the light's xz-plane and then normalize
                const Vector3& w_horizontal = (wi - wi.dot(lightFrame.rightVector()) * lightFrame.rightVector()).direction();
                const Vector3& w_vertical   = (wi - wi.dot(lightFrame.upVector())    * lightFrame.upVector()).direction();

                // Now test against the view cone in each of the planes 
                return 
                    (w_horizontal.dot(lightFrame.lookVector()) < threshold) &&
                    (w_vertical.dot(lightFrame.lookVector())   < threshold);
            } else {
                // Test against a cone
                return wi.dot(lightFrame.lookVector()) < threshold;
            }
        }

    case Type::AREA:
        return true;

    default:
        alwaysAssertM(false, "Invalid light type for Light::inFieldOfView()");
        return false;
    }
}


Biradiance3 Light::biradiance(const Point3& X) const {
    return biradiance(X, frame().translation);
}

Biradiance3 Light::biradiance(const Point3& X, const Point3& P) const {
    Vector3 wi = P - X;
    float d = wi.length();
    wi /= d;

    // Attempt to enforce a minimum distance in the computation
    // d = max(d, min(m_extent.x, m_extent.y));

    const Biradiance3& b = bulbPower() / (4.0f * pif() * (attenuation[0] + attenuation[1] * d + attenuation[2] * square(d)));

    if (m_type == Type::SPOT) {
        return b * spotLightFalloff(wi);
    } else if (m_type == Type::AREA) {
        return b * max(0.0f, -wi.dot(m_frame.lookVector()));
    } else {
        return b;
    }
}


void Light::getRectangularAreaLightVertices(Point3 p[4]) const {
    alwaysAssertM((type() == Type::AREA) && m_rectangular, "Not a rectangular area light");
    p[0] = m_frame.pointToWorldSpace(Point3(-0.5f * m_extent.x, -0.5f * m_extent.y, 0.0f));
    p[1] = m_frame.pointToWorldSpace(Point3(+0.5f * m_extent.x, -0.5f * m_extent.y, 0.0f));
    p[2] = m_frame.pointToWorldSpace(Point3(+0.5f * m_extent.x, +0.5f * m_extent.y, 0.0f));
    p[3] = m_frame.pointToWorldSpace(Point3(-0.5f * m_extent.x, +0.5f * m_extent.y, 0.0f));
}


float Light::spotLightFalloff(const Vector3& w_i)  const {

    float cosHalfAngle, softnessConstant;
    getSpotConstants(cosHalfAngle, softnessConstant);

    const Vector3& look  = frame().lookVector();
    const Vector3& up    = frame().upVector();
    const Vector3& right = frame().rightVector();

    float s = clamp((-look.dot(w_i) - cosHalfAngle) * softnessConstant, 0.0f, 1.0f);

    // When the light field of view is very small, we need to be very careful with precision 
    // on the computation below, so there are epsilon values in the comparisons.
    if (m_rectangular) {
        // Project wi onto the light's xz-plane and then normalize
        Vector3 w_horizontal = normalize(w_i - w_i.dot(right) * right);
        Vector3 w_vertical   = normalize(w_i - w_i.dot(up)    * up);

        // Now test against the view cone in each of the planes 
        s *= float(w_horizontal.dot(look) <= -cosHalfAngle + 1e-5f) *
             float(w_vertical.dot(look) <= -cosHalfAngle + 1e-5f);
    }

    return s;
}

Point2 Light::lowDiscrepancySample(int pixelIndex, int lightIndex, int sampleIndex, int numSamples) const {
    const int f[] = { pixelIndex, lightIndex };
    const uint32_t hash = superFastHash(&f, 2 * sizeof(int));
    // Normalize both halves of the 32 bit hash value into 2 component 0-1 vector.
    const Point2 shift((hash >> 16)* (1.0f / float(0xFFFF)), (hash & 0xFFFF)* (1.0f / float(0xFFFF)));
    const Point2& h = (Point2::hammersleySequence2D(sampleIndex, numSamples) + shift).mod1();
    return h;
}

Point3 Light::lowDiscrepancyAreaPosition(int pixelIndex, int lightIndex, int sampleIndex, int numSamples) const {
    Point2 sample = lowDiscrepancySample(pixelIndex, lightIndex, sampleIndex, numSamples);
    return position(sample.x * 2.0f - 1.0f, sample.y * 2.0f - 1.0f).xyz();
}

Point3 Light::uniformAreaPosition() const {
    Random& rng = Random::threadCommon();
    float u = rng.uniform();
    float v = rng.uniform();
    return position(u * 2.0f - 1.0f, v * 2.0f - 1.0f).xyz();
}

Point3 Light::stratifiedAreaPosition(int pixelIndex, int sampleIndex, int numSamples) const {

    // Minimum divisor of numSamples greater sqrt(numSamples).
    int horizontalStrata = iCeil(sqrtf((float)numSamples));
    while (numSamples % horizontalStrata != 0) {
        ++horizontalStrata;
    }
    int verticalStrata = numSamples / horizontalStrata;

    Random& rng = Random::threadCommon();

    // Flip the strata axes at every other pixel in case
    // numSamples is (nearly) prime.
    if (pixelIndex % 2 == 0) {
        int temp = horizontalStrata;
        horizontalStrata = verticalStrata;
        verticalStrata = temp;
    }

    const int stratumIndex = sampleIndex % (horizontalStrata * verticalStrata);

    float u = rng.uniform();
    float v = rng.uniform();

    // Place a smple within the next stratum.
    u = (u + float(stratumIndex % horizontalStrata)) / float(horizontalStrata);
    v = (v + float(stratumIndex / horizontalStrata)) / float(verticalStrata);

    return position(u * 2.0f - 1.0f, v * 2.0f - 1.0f).xyz();
}

Vector3 Light::sampleSphericalQuad(const Point3& origin, float u, float v, float& solidAngle) const {
        // compute local reference system ’R’
        const Vector3& X = -m_frame.rightVector();
        const Vector3& Y = m_frame.upVector();
        Vector3 Z = m_frame.lookVector();

        // compute rectangle coords in local reference system
        Vector3 dVec = m_frame.pointToWorldSpace(Point3(0.5f * m_extent.x, -0.5f * m_extent.y, 0.0f)) - origin;
        float z0 = dot(dVec, Z);

        // flip ’z’ to make it point against ’Q’
        if (z0 > 0) {
            // Because of the way we set up the coordinate system this
            // branch is never taken for single sided light sources
            // that face toward the origin.
            Z *= -1;
            z0 *= -1;
        }

        float x0 = dot(dVec, X);
        float y0 = dot(dVec, Y);
        float x1 = x0 + m_extent.x;
        float y1 = y0 + m_extent.y;

        // create vectors to four vertices
        Vector3 v00 = { x0, y0, z0 };
        Vector3 v01 = { x0, y1, z0 };
        Vector3 v10 = { x1, y0, z0 };
        Vector3 v11 = { x1, y1, z0 };

        // compute normals to edges
        Vector3 n0 = normalize(cross(v00, v10));
        Vector3 n1 = normalize(cross(v10, v11));
        Vector3 n2 = normalize(cross(v11, v01));
        Vector3 n3 = normalize(cross(v01, v00));

        // compute internal angles (gamma_i)
        float g0 = acos(-dot(n0, n1));
        float g1 = acos(-dot(n1, n2));
        float g2 = acos(-dot(n2, n3));
        float g3 = acos(-dot(n3, n0));

        // compute solid angle from internal angles
        float k = 2 * pif() - g2 - g3;

        // Avoid 0 or negative solid angle.
        solidAngle = max(0.00001f, g0 + g1 - k);

        // Sample the spherical quad
        const float eps = 1e-4;
        // 1. compute ’cu’
        float au = u * solidAngle + k;
        float fu = (cos(au) * n0.z - n2.z) / sin(au);
        float cu = 1 / sqrt(fu * fu + n0.z * n0.z) * (fu > 0 ? +1 : -1);
        cu = clamp(cu, -1.0f, 1.0f); // avoid NaNs
        // 2. compute ’xu’
        float xu = -(cu * z0) / sqrt(1 - cu * cu);
        xu = clamp(xu, x0, x1); // avoid Infs
        // 3. compute ’yv’
        float d = sqrt(xu * xu + z0 * z0);
        float h0 = y0 / sqrt(d * d + y0 * y0);
        float h1 = y1 / sqrt(d * d + y1 * y1);
        float hv = h0 + v * (h1 - h0), hv2 = hv * hv;
        float yv = (hv2 < 1 - eps) ? (hv * d) / sqrt(1 - hv2) : y1;

        // 4. transform (xu,yv,z0) to world coords
        return (origin + xu * X + yv * Y + z0 * Z);
}

Point3 Light::lowDiscrepancySolidAnglePosition(int pixelIndex, int lightIndex, int sampleIndex, int numSamples, const Point3& X, float& areaTimesPDFValue) const {
    Point2 areaSample = lowDiscrepancySample(pixelIndex, lightIndex, sampleIndex, numSamples);

    float solidAngle;
    Point3 P = sampleSphericalQuad(X, areaSample.x, areaSample.y, solidAngle);

    Vector3 wi = P - X;
    float d = wi.length();
    wi /= d;

    // This is the pdf over the light area of a point uniformly sampled in solid angle space.
    // The same pdf over the solid angle would be constant: 1 / solidAngle.
    // Note the absolute value to ensure that we return the correct pdf for points behind the area light.
    // For pathtracing, the biradiance will be 0 for such points, so the answer will still be correct.
    float pdfValue = (fabs(wi.dot(m_frame.lookVector())) / (attenuation[0] + attenuation[1] * d + attenuation[2] * square(d))) * (1.0f / solidAngle);

    areaTimesPDFValue = (m_extent.x * m_extent.y) * pdfValue;

    return P;
}


Vector3 Light::randomEmissionDirection(Random& rng) const {
    if (m_type == Type::SPOT) {
        if (rectangular()) {
            Vector3 v;
            do {
                v = Vector3::random(rng);
            } while (!inFieldOfView(-v.direction()));
            return v.direction();
        } else {
            Cone spotCone(Point3(), m_frame.lookVector(), spotHalfAngle());
            return spotCone.randomDirectionInCone(rng);
        }
    } if (m_type == Type::DIRECTIONAL) {
        return m_frame.lookVector();
    } else {
        return Vector3::random(rng);
    }  
}


void Light::onSimulation(SimTime absoluteTime, SimTime deltaTime) {
    VisibleEntity::onSimulation(absoluteTime, deltaTime);

    static const float radius = extent().length() / 2.0f;
    static const float dirDist = 1000.0f;
    switch(m_type){

    case Type::SPOT:
    case Type::OMNI:
        m_lastSphereBounds  = Sphere(position().xyz(), radius);
        m_lastSphereBounds.getBounds(m_lastAABoxBounds);
        m_lastBoxBoundArray.resize(1);
        m_lastBoxBoundArray[0] = m_lastBoxBounds = m_lastAABoxBounds;
        break;

    case Type::AREA:
        m_lastSphereBounds  = Sphere(position().xyz(), radius);
        {
            const AABox osBounds(Point3(-m_extent.x * 0.5f, -m_extent.y * 0.5f, 0.0f), Point3(m_extent.x * 0.5f, m_extent.y * 0.5f, 0.0f)); 
            m_lastBoxBounds = m_frame.toWorldSpace(osBounds);
            m_lastBoxBoundArray.resize(1);
            m_lastBoxBoundArray[0] = m_lastBoxBounds;
            m_lastBoxBounds.getBounds(m_lastAABoxBounds);
        }
        break;

    case Type::DIRECTIONAL:
        m_lastSphereBounds  = Sphere(position().xyz() * dirDist, radius * dirDist);        
        m_lastSphereBounds.getBounds(m_lastAABoxBounds);
        m_lastBoxBoundArray.resize(1);
        m_lastBoxBoundArray[0] = m_lastBoxBounds = m_lastAABoxBounds;
        break;

    default:
        alwaysAssertM(false, "Invalid light type");
        break;
    }

    
    if (m_bulbPowerTrack.size() > 0) {
        const Color3& c = max(m_bulbPowerTrack.evaluate((float)absoluteTime), Color3::zero());
        if (color != c) {
            m_lastChangeTime = System::time();
        }
        color = c;
    }
}


void Light::onPose(Array<shared_ptr<Surface> >& surfaceArray) {
    if (visible()) {
        if (isNull(m_model) && (m_type == Light::Type::AREA)) {
            // Create geometry

            // Note: we could cache the parameters used to create the model to
            // support allowing runtime changes to a light that will be reflected
            // by its generated geometry.

            const shared_ptr<ArticulatedModel>& model = ArticulatedModel::createEmpty("generatedLightGeometry");

            ArticulatedModel::Part*     part      = model->addPart("root");
            ArticulatedModel::Geometry* geometry  = model->addGeometry("geom");
            ArticulatedModel::Mesh*     mesh      = model->addMesh("mesh", part, geometry);

            const Color4unorm8 data[16] = {
                Color4unorm8(Color3::zero()), Color4unorm8(Color3::zero()), Color4unorm8(Color3::zero()), Color4unorm8(Color3::zero()),
                Color4unorm8(Color3::zero()), Color4unorm8(Color3::zero()), Color4unorm8(Color3::zero()), Color4unorm8(Color3::zero()),

                Color4unorm8(Color3::one()), Color4unorm8(Color3::one()), Color4unorm8(Color3::one()), Color4unorm8(Color3::one()),
                Color4unorm8(Color3::one()), Color4unorm8(Color3::one()), Color4unorm8(Color3::one()), Color4unorm8(Color3::one())
            };
            const shared_ptr<GLPixelTransferBuffer>& ptb = GLPixelTransferBuffer::create(4, 4, ImageFormat::RGBA8(), data);

            UniversalMaterial::Specification matSpec;

            // The 4.0 in the denominator corresponds to a light emitting over a *sphere* based on that bulbPower().
            // G3D SPOT lights are defined to have a bulb power that ignores the barn doors (which is handy for this computation
            // because we don't have to adjust for the solid angle of the barn doors in computing the emissive value).
            // For an area light, you might expect to NOT see the 4.0 in the denominator, since the area light doesn't shine out its
            // back and experiences a cosine-weighted hemisphere. Thus the traditional computation for an area light is
            // power / (pi * area), and not power / (4*pi*area). However, the way that G3D area lights are specified
            // and implemented by the biradiance() function takes this into account. It is just an issue of the convention
            // for specifying light intensity, not radiometry.

            const Radiance3 bulbRadiance = bulbPower() / (4.0f * pif() * max(0.01f, extent().x * extent().y));
            matSpec.setLambertian(Texture::opaqueBlack());//Texture::fromPixelTransferBuffer("G3D::Light Surface Lambertian Texture", ptb, Texture::Encoding(ptb->format(), FrameName::NONE, bulbRadiance * 0.1f / clamp(bulbRadiance.max(), 0.0f, 1.0f))));
            matSpec.setEmissive(Texture::fromPixelTransferBuffer("G3D::Light Surface Emissive Texture", ptb, Texture::Encoding(ptb->format(), FrameName::NONE, bulbRadiance)));
            matSpec.removeGlossy();
            matSpec.setFlags(1);
            mesh->material = UniversalMaterial::create(matSpec);

            Array<CPUVertexArray::Vertex>& vertexArray = geometry->cpuVertexArray.vertex;
            Array<int>& indexArray = mesh->cpuIndexArray;

            // Front
            const Point2 white(0.5f, 0.75f);
            vertexArray.append
               (CPUVertexArray::Vertex(Point3(m_extent.x *  0.5f, m_extent.y *  0.5f, 0.0f), white),
                CPUVertexArray::Vertex(Point3(m_extent.x *  0.5f, m_extent.y * -0.5f, 0.0f), white),
                CPUVertexArray::Vertex(Point3(m_extent.x * -0.5f, m_extent.y * -0.5f, 0.0f), white),
                CPUVertexArray::Vertex(Point3(m_extent.x * -0.5f, m_extent.y *  0.5f, 0.0f), white));

            indexArray.append(0, 1, 2, 0, 2, 3);

            // Back
            const Point2 black(0.5f, 0.25f);
            vertexArray.append
               (CPUVertexArray::Vertex(Point3(m_extent.x *  0.5f, m_extent.y *  0.5f, 0.0f), black),
                CPUVertexArray::Vertex(Point3(m_extent.x *  0.5f, m_extent.y * -0.5f, 0.0f), black),
                CPUVertexArray::Vertex(Point3(m_extent.x * -0.5f, m_extent.y * -0.5f, 0.0f), black),
                CPUVertexArray::Vertex(Point3(m_extent.x * -0.5f, m_extent.y *  0.5f, 0.0f), black));
            indexArray.append(6, 5, 4, 7, 6, 4);

            // Tell the ArticulatedModel to generate bounding boxes, GPU vertex arrays,
            // normals and tangents automatically. We already ensured correct
            // topology, so avoid the vertex merging optimization.
            ArticulatedModel::CleanGeometrySettings geometrySettings;
            geometrySettings.allowVertexMerging = false;
            model->cleanGeometry(geometrySettings);

            // Install the model
            setModel(model);
        }
    }

    VisibleEntity::onPose(surfaceArray);
}


Any Light::toAny(const bool forceAll) const {
    
    Any a = VisibleEntity::toAny(forceAll);
    a.setName("Light");
    
    // check if the color has changed if it has change it
    AnyTableReader oldValues(a);
    Color3 temp;
    oldValues.getIfPresent("biradiance", temp);
    oldValues.getIfPresent("bulbPower", temp);
    if (temp != color) { 
        a["bulbPower"] = color;
    }
    
    //check if the attenuation has changed. if it has change it
    bool setAttenuation = false;
    Array<float> tempAtt = { 0.0001f, 0.0f, 1.0f };
    oldValues.getIfPresent("attenuation",  tempAtt);
    for (int i = 0; i < 3; ++i) {
        if (attenuation[i] != tempAtt[i]) {
            setAttenuation = true;
        }
    }
    if (setAttenuation) {
        a["attenuation"] = tempAtt; 
    }

    float spotHalfAngleDegrees = 180.0f;
    oldValues.getIfPresent("spotHalfAngleDegrees", spotHalfAngleDegrees);
    if (m_spotHalfAngle != toRadians(spotHalfAngleDegrees) ) {
        a["spotHalfAngleDegrees"] = toDegrees(m_spotHalfAngle);
    }

    float areaLightPullback = 0.1f;
    oldValues.getIfPresent("areaLightPullback", areaLightPullback);
    if (areaLightPullback != m_areaLightPullback) {
        a["areaLightPullback"] = m_areaLightPullback;
    }

    float spotHardness = 1.0f;
    oldValues.getIfPresent("spotHardness", spotHardness);
    if (m_spotHardness != spotHardness) {
        a["spotHardness"] = m_spotHardness;
    }

    bool shadowsEnabled = false;
    oldValues.getIfPresent("shadowsEnabled", shadowsEnabled);
    if (m_shadowsEnabled != shadowsEnabled) {
        a["shadowsEnabled"] = m_shadowsEnabled; 
    }

    ShadowMap::VSMSettings vsmSettings;
    oldValues.getIfPresent("varianceShadowSettings", vsmSettings);
    if (m_varianceShadowSettings != vsmSettings) {
        a["varianceShadowSettings"] = vsmSettings;
    }

    bool enabled = true;
    oldValues.getIfPresent("enabled", enabled); 
    if (m_enabled != enabled) {
        a["enabled"] = m_enabled;
    }

    bool producesIndirect = true;
    oldValues.getIfPresent("producesIndirectIllumination", producesIndirect);
    if ( m_producesIndirectIllumination != producesIndirect ) {
        a["producesIndirectIllumination"] = m_producesIndirectIllumination;
    }

    bool producesDirect  = true;
    oldValues.getIfPresent("producesDirectIllumination", producesDirect);
    if ( m_producesDirectIllumination != producesDirect ) {
        a["producesDirectIllumination"] = m_producesDirectIllumination;
    }

    bool rect  = true;
    oldValues.getIfPresent("rectangular", rect);
    if ( m_rectangular != rect ) {
        a["rectangular"] = m_rectangular;
    }

    return a;
}

    
Light::Light() :
    m_type(Type::DIRECTIONAL),
    m_spotHalfAngle(pif()),
    m_rectangular(false),
    m_spotHardness(1.0f),
    m_shadowsEnabled(true),
    m_varianceShadowSettings(),
    m_shadowCullFace(CullFace::BACK),
    m_enabled(true),
    m_extent(0.2f, 0.2f),
    m_producesIndirectIllumination(true),
    m_producesDirectIllumination(true),
    m_nearPlaneZLimit(-0.01f),
    m_farPlaneZLimit(-finf()),
    color(Color3::white()) {

    // Directional light attenuation
    attenuation[0]  = 0.0001f;
    attenuation[1]  = 0.0f;
    attenuation[2]  = 1.0f;
}


void Light::setFrame(const CoordinateFrame& c, bool updatePreviousFrame) {
    VisibleEntity::setFrame(c, updatePreviousFrame);
    if (m_type == Type::DIRECTIONAL) {
        m_frame.translation = -m_frame.lookVector() * 10000.0f;
    }
}


shared_ptr<Light> Light::directional(const String& name, const Vector3& toLight, const Color3& color, bool s, int shadowMapRes) {
    
    shared_ptr<Light> L = createShared<Light>();
    L->m_name = name;
    L->m_type = Type::DIRECTIONAL;

    CFrame c;
    c.lookAt(-toLight.direction());
    L->setFrame(c);

    L->color    = color;
    L->m_shadowsEnabled = s;
    if (L->m_shadowsEnabled && (shadowMapRes > 0)) {
        L->m_shadowMap = ShadowMap::create(name + " shadow map", Vector2int16(shadowMapRes, shadowMapRes));
    }
    return L;
}


shared_ptr<Light> Light::point(const String& name, const Vector3& pos, const Color3& color, float constAtt, float linAtt, float quadAtt, bool s, int shadowMapRes) {
    
    shared_ptr<Light> L(new Light());

    L->m_name = name;
    L->m_type = Type::OMNI;
    L->m_frame.translation = pos;
    L->color    = color;
    L->attenuation[0] = constAtt;
    L->attenuation[1] = linAtt;
    L->attenuation[2] = quadAtt;
    L->m_shadowsEnabled   = s;
    if (L->m_shadowsEnabled && (shadowMapRes > 0)) {
        L->m_shadowMap = ShadowMap::create(name + " shadow map", Vector2int16(shadowMapRes, shadowMapRes));
    }
    L->computeFrame(-Vector3::unitZ(), Vector3::unitX());

    return L;
    
}


shared_ptr<Light> Light::spot(const String& name, const Vector3& pos, const Vector3& pointDirection, float spotHalfAngle, const Color3& color, float constAtt, float linAtt, float quadAtt, bool s, int shadowMapRes) {
    
    shared_ptr<Light> L(new Light());
    L->m_name         = name;
    L->m_type         = Type::SPOT;
    L->m_frame.translation = pos;
    debugAssert(spotHalfAngle <= pif() / 2.0f);
    L->m_spotHalfAngle  = spotHalfAngle;
    L->color          = color;
    L->attenuation[0] = constAtt;
    L->attenuation[1] = linAtt;
    L->attenuation[2] = quadAtt;
    L->m_shadowsEnabled   = s;
    if (L->m_shadowsEnabled && (shadowMapRes > 0)) {
        L->m_shadowMap = ShadowMap::create(name + " shadow map", Vector2int16(shadowMapRes, shadowMapRes));
    }
    L->computeFrame(pointDirection.direction(), Vector3::zero());

    return L;

}


bool Light::possiblyIlluminates(const class Sphere& objectSphere) const {
    if (m_type == Type::DIRECTIONAL) {
        return true;
    }

    // OMNI and SPOT cases
    const Sphere& lightSphere = effectSphere();

    if (! lightSphere.intersects(objectSphere)) {
        // The light can't possibly reach the object
        return false;
    } else if (m_type == Type::OMNI) {
        return true;
    }

    // SPOT case
    const Cone spotCone(m_frame.translation, m_frame.lookVector(), spotHalfAngle());
    return spotCone.intersects(objectSphere);
}


Sphere Light::effectSphere(float cutoff) const {
    if (m_type == Type::DIRECTIONAL) {
        // Directional light
        return Sphere(Vector3::zero(), finf());
    } else {
        // Avoid divide by zero
        cutoff = max(cutoff, 0.00001f);
        float maxIntensity = max(color.r, max(color.g, color.b));

        float radius = finf();
            
        if (attenuation[2] != 0) {

            // Solve I / attenuation.dot(1, r, r^2) < cutoff for r
            //
            //  a[0] + a[1] r + a[2] r^2 > I/cutoff
            //
            
            float a = attenuation[2];
            float b = attenuation[1];
            float c = attenuation[0] - maxIntensity / cutoff;
            
            float discrim = square(b) - 4 * a * c;
            
            if (discrim >= 0) {
                discrim = sqrt(discrim);
                
                float r1 = (-b + discrim) / (2 * a);
                float r2 = (-b - discrim) / (2 * a);

                if (r1 < 0) {
                    if (r2 > 0) {
                        radius = r2;
                    }
                } else if (r2 > 0) {
                    radius = min(r1, r2);
                } else {
                    radius = r1;
                }
            }
                
        } else if (attenuation[1] != 0) {
            
            // Solve I / attenuation.dot(1, r) < cutoff for r
            //
            // r * a[1] + a[0] = I / cutoff
            // r = (I / cutoff - a[0]) / a[1]

            float radius = (maxIntensity / cutoff - attenuation[0]) / attenuation[1];
            radius = max(radius, 0.0f);
        }

        return Sphere(m_frame.translation, radius);

    }
}


void Light::computeFrame(const Vector3& spotDirection, const Vector3& rightDirection) {
    if (rightDirection == Vector3::zero()) {
        // No specified right direction; choose one automatically
        if (m_type == Type::DIRECTIONAL) {
            // Directional light
            m_frame.lookAt(-m_frame.translation);
        } else {
            // Spot light
            m_frame.lookAt(m_frame.translation + spotDirection);
        }
    } else {
        const Vector3& Z = -spotDirection.direction();
        Vector3 X = rightDirection.direction();

        // Ensure the vectors are not too close together
        while (abs(X.dot(Z)) > 0.9f) {
            X = Vector3::random();
        }

        // Ensure perpendicular
        X -= Z * Z.dot(X);
        const Vector3& Y = Z.cross(X);

        m_frame.rotation.setColumn(Vector3::X_AXIS, X);
        m_frame.rotation.setColumn(Vector3::Y_AXIS, Y);
        m_frame.rotation.setColumn(Vector3::Z_AXIS, Z);
    }
}


void Light::getSpotConstants(float& cosHalfAngle, float& softnessConstant) const {
    cosHalfAngle = -1.0f;

    if (spotHalfAngle() < pif()) {
        // Spot light
        const float angle = spotHalfAngle();
        cosHalfAngle = cos(angle);
    }

    const float epsilon = 1e-10f;
    softnessConstant = 1.0f / ((1.0f - m_spotHardness + epsilon) * (1.0f - cosHalfAngle));
}


void Light::setShaderArgs(UniformTable& args, const String& prefix) const {
    Vector4 P = position();

    if (type() == Type::AREA) {
        P = Vector4(P.xyz() - m_areaLightPullback * frame().lookVector() * P.w, P.w);
    }

    args.setUniform(prefix + "position",    P);
    args.setUniform(prefix + "color",       color);
    args.setUniform(prefix + "rectangular", m_rectangular);
    args.setUniform(prefix + "direction",   frame().lookVector());
    args.setUniform(prefix + "up",          frame().upVector());
    args.setUniform(prefix + "right",       frame().rightVector());
    
    float cosHalfAngle, softnessConstant;
    getSpotConstants(cosHalfAngle, softnessConstant);
    
    args.setUniform(prefix + "attenuation",
        Vector4(attenuation[0],
            attenuation[1],
            attenuation[2],
            cosHalfAngle));

    args.setUniform(prefix + "softnessConstant", softnessConstant);

    const float lightRadius = m_extent.length() / 2.0f;
    args.setUniform(prefix + "radius", lightRadius);

    // The last character is either "." for struct-style passing or "_" for underscore-style
    // uniform passing. Repeat whichever convention was used for the light for the 
    // shadow map fields.
    const String& separatorChar = prefix.empty() ? "_" : prefix.substr(prefix.length() - 1);

    if (shadowsEnabled()) {
        shadowMap()->setShaderArgsRead(args, prefix + "shadowMap" + separatorChar);
    }
}


float Light::nearPlaneZ() const {
    if (notNull(m_shadowMap)) {
        return m_shadowMap->projection().nearPlaneZ();
    } else {
        return m_nearPlaneZLimit;
    }
}


float Light::farPlaneZ() const {
    if (notNull(m_shadowMap)) {
        return m_shadowMap->projection().farPlaneZ();
    } else {
        return m_farPlaneZLimit;
    }
}


float Light::nearPlaneZLimit() const {
    return m_nearPlaneZLimit;
}


float Light::farPlaneZLimit() const {
    return m_farPlaneZLimit;
}


void Light::makeGUI(GuiPane* pane, GApp* app) {
    VisibleEntity::makeGUI(pane, app);
    pane->addCheckBox("Enabled", Pointer<bool>(this, &Light::enabled, &Light::setEnabled));
    pane->addCheckBox("Shadows Enabled", Pointer<bool>(this, &Light::shadowsEnabled, &Light::setShadowsEnabled));
}


void Light::renderShadowMaps(RenderDevice* rd, const Array<shared_ptr<Light> >& lightArray, const Array<shared_ptr<Surface> >& allSurfaces, CullFace cullFace) {
    BEGIN_PROFILER_EVENT("Light::renderShadowMaps");

    // Find the scene bounds
    AABox shadowCasterBounds;
    bool ignoreBool;
    Surface::getBoxBounds(allSurfaces, shadowCasterBounds, false, ignoreBool, true);

    Array<shared_ptr<Surface> > lightVisible;
    lightVisible.reserve(allSurfaces.size());

    // Generate shadow maps
    int s = 0;
    for (const shared_ptr<Light>& light : lightArray) {
        
        if (light->shadowsEnabled() && light->enabled()) {

            CFrame lightFrame;
            Matrix4 lightProjectionMatrix;
                
            const float nearMin = -light->nearPlaneZLimit();
            const float farMax = -light->farPlaneZLimit();
            ShadowMap::computeMatrices(light, shadowCasterBounds, lightFrame, light->shadowMap()->projection(), lightProjectionMatrix, 20, 20, nearMin, farMax);
                
            // Cull objects not visible to the light
            debugAssert(notNull(light->shadowMap()->depthTexture()));
            Surface::cull(lightFrame, light->shadowMap()->projection(), light->shadowMap()->rect2DBounds(), allSurfaces, lightVisible, false);

            // Cull objects that don't cast shadows
            for (int i = 0; i < lightVisible.size(); ++i) {
                if (! lightVisible[i]->expressiveLightScatteringProperties.castsShadows) {
                    // Changing order is OK because we sort below
                    lightVisible.fastRemove(i);
                    --i;
                }
            }

            Surface::sortFrontToBack(lightVisible, lightFrame.lookVector());

            const CullFace renderCullFace = (cullFace == CullFace::CURRENT) ? light->shadowCullFace() : cullFace;
            const Color3 transmissionWeight = light->bulbPower() / max(light->bulbPower().sum(), 1e-6f);

            if (light->shadowMap()->useVarianceShadowMap()) {
                light->shadowMap()->updateDepth(rd, lightFrame, lightProjectionMatrix, lightVisible, renderCullFace, transmissionWeight, RenderPassType::OPAQUE_SHADOW_MAP);
                light->shadowMap()->updateDepth(rd, lightFrame, lightProjectionMatrix, lightVisible, renderCullFace, transmissionWeight, RenderPassType::TRANSPARENT_SHADOW_MAP);
            } else {
                light->shadowMap()->updateDepth(rd, lightFrame, lightProjectionMatrix, lightVisible, renderCullFace, transmissionWeight, RenderPassType::SHADOW_MAP);
            }

            lightVisible.fastClear();
            ++s;
        }
    }

    END_PROFILER_EVENT();
}

} // G3D
