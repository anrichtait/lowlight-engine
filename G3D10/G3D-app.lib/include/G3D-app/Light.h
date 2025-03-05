/**
  \file G3D-app.lib/include/G3D-app/Light.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define GLG3D_Light_h

#include "G3D-base/platform.h"
#include "G3D-base/Vector4.h"
#include "G3D-base/Vector3.h"
#include "G3D-base/Color4.h"
#include "G3D-base/Array.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/CoordinateFrame.h"
#include "G3D-base/enumclass.h"
#include "G3D-base/Spline.h"
#include "G3D-app/Scene.h"
#include "G3D-app/VisibleEntity.h"
#include "G3D-app/ShadowMap.h"

namespace G3D {
class Any;
class Scene;
class Surface;

typedef Spline<Power3> Power3Spline;

/**
   \brief An (invisible) emitting surface (AREA) or point (DIRECTIONAL, SPOT, OMNI) light.

   The light "faces" along the negative-z axis of its frame(), like
   all other G3D objects.

   The light properties do not change when it is disabled (except for the enabled() value, of course).
   The caller is responsible for ensuring that lights are enabled when using them.

   For parameterizing the light's power, imagine putting a bulb with a given bulbPower()
   in a light fixture. (Keep in mind that bulbs you buy in a store are marked with their equivalent
   power *consumption* for a ~5% efficient incandescent, so a "100W" bulb is really a "5W" emitter.)
   The shape of the light fixture determines the emittedPower():

   - The OMNI fixture is a perfect diffusing sphere. Integrated over all directions, the total power emitted is the bulbPower.
   - The SPOT fixture is a perfect diffusing sphere in a thin box that is perfectly black on the 
      back side and has black barn doors. The total power emitted is proportional to the solid angle of the door aperture.
      unless the spotLightFalloff value is used.
   - The AREA fixture is a thin box that is black on the back and perfectly diffusing on the bottom. The total power emitted 
      is 1/4 of the bulb power (1/2 the sphere and then cosine falloff for the planar instead of hemisphere emitter).
      If you specify a spot angle on an area source then it also is limited by the solid angle of the door aperture.

   This conceptual model of luminaires makes it easy to adjust light types in a scene. Turning an omni light into a spot light
   or a (small) area light preserves the observed intensity of objects directly under the light source. Be careful when comparing
   to published results; in many cases a rendering paper describes the emittedPower instead of the bulbPower, so a "1m^2 1W area light"
   would be a "1m^2 4w bulb area light" in G3D.

   For reading from an Any, the following fields are supported:
       
   \code
    Light {
        shadowsEnabled = bool;
        shadowMapSize = Vector2int16(w, h);
        shadowMapBias = float;   // In meters, default is zero. Larger creates dark leaks, smaller creates light leaks
        shadowCullFace = CullFace;  // may not be CURRENT
        stochasticShadows = bool;
        varianceShadowSettings = VSMSettings {};
        enabled      = bool;
        rectangular  = bool;
        areaLightPullback = float; // in meters, how far point light approximations should pull the area light back to improve the approximation
        spotHardness = float; // 0 = cosine falloff, 1 (default) = step function falloff
        attenuation  = [number number number];
        bulbPower    = Power3; (for a spot or omni light)
        bulbPowerTrack = Power3Spline { ... };
        biradiance   = Biradiance3; (for a directional light)
        type         = "DIRECTIONAL", "SPOT", "OMNI", or "AREA";
        spotHalfAngleDegrees = number;
        producesIndirectIllumination = boolean;
        producesDirectIllumination = boolean;

        nearPlaneZLimit = number; (negative)
        farPlaneZLimit = number; (negative)
        ... // plus all VisibleEntity properties
    }
    \endcode

    plus all G3D::Entity fields. 


   A directional light has position.w == 0.  A spot light has
   spotHalfAngle < pi() / 2 and position.w == 1.  An omni light has
   spotHalfAngle == pi() and position.w == 1.  

   For a physically correct light, set attenuation = (0,0,1) for SPOT
   and OMNI lights (the default).  UniversalSurface ignores attenuation on
   directional lights, although in general it should be (1,0,0).
   
   \image html normaloffsetshadowmap.png
 */
class Light : public VisibleEntity {
public:

    class Type {
    public:
        enum Value {
            /**
             A "wall of lasers" approximating an infinitely distant, very bright
             SPOT light.  This provides constant incident radiance from a single 
             direction everywhere in the scene.

             Distance Attenuation is not meaningful on directional lights.
             */
            DIRECTIONAL,

            /**
              An omni-directional point light within a housing that only allows
              light to emerge in a cone (or frustum, if square).
             */
            SPOT, 

            /** 
              An omni-directional point light that emits in all directions.
              G3D does not provide built-in support shadow maps for omni lights.
            */
            OMNI,

            /** Reserved for future use. */
            AREA
        } value;
    
        static const char* toString(int i, Value& v) {
            static const char* str[] = {"DIRECTIONAL", "SPOT", "OMNI", "AREA", nullptr};
            static const Value val[] = {DIRECTIONAL, SPOT, OMNI, AREA};             
            const char* s = str[i];
            if (s) {
                v = val[i];
            }
            return s;
        }

        G3D_DECLARE_ENUM_CLASS_METHODS(Type);
    };

protected:

    Type                m_type;

    /** Spotlight cutoff half-angle in <B>radians</B>.  pi() = no
        cutoff (point/dir).  Values less than pi()/2 = spot light */
    float               m_spotHalfAngle;

    /** If true, setShaderArgs will bind a spotHalfAngle large
        enough to encompass the entire square that bounds the cutoff
        angle. This produces a frustum instead of a cone of light
        when used with a G3D::ShadowMap.  for an unshadowed light this
        has no effect.*/
    bool                m_rectangular;

    /** 1 = hard cutoff (default). 0 = cosine falloff within cone (like photoshop's brush hardness)
       
       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       float t = dir.dot(lightDir) - cosHalfAngle;       
       t  /= 1 - cosHalfAngle; // On [0, 1]
       float softness = (1 - hardness);

       // Avoid Nan from 0/0
       float brightness = clamp(t / (softness + eps), 0, 1);

       //////////////////////////////////////////
       // Using precomputed values:
       const float lightSoftnessConstant = 1.0 / ((1 - hardness + eps) * (1 - cosHalfAngle));
       float brightness = clamp((dir.dot(lightDir) - cosHalfAngle) * lightSoftnessConstant, 0, 1);
       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
      */
    float               m_spotHardness;

    float               m_areaLightPullback = 0.1f;

    bool                m_shadowsEnabled;

    ShadowMap::VSMSettings m_varianceShadowSettings;

    CullFace            m_shadowCullFace;

    /** If false, this light is ignored */
    bool                m_enabled;

    /** 
     Optional shadow map.
    */
    shared_ptr<ShadowMap> m_shadowMap;

    /** \copydoc Light::extent() */
    Vector2             m_extent;

    /** If set, this is used in onSimulation */
    Power3Spline        m_bulbPowerTrack;

    bool                m_producesIndirectIllumination;
    bool                m_producesDirectIllumination;
    
    float               m_nearPlaneZLimit;
    float               m_farPlaneZLimit;

public:

    /** The attenuation observed by an omni or spot light is 

        \f[ \frac{1}{4 \pi (a_0 + a_1 r + a_2 r^2)},\f]

        where \f$a_i\f$ <code>= attenuation[i]</code> and
        \f$r\f$ is the distance to the source.
    
        Directional lights ignore attenuation.  A physically correct
        light source should have $\f$a_0=0, a_1=0, a_2=1\f$, but it
        may be artistically desirable to alter the falloff function.

        To create a local light where the biradiance is equal to 
        the bulbPower with "no attenuation", use $\f$a_0=1/(4\pi), a_1=0, a_2=0\f$.
    */
    float               attenuation[3];

    /** 
        Point light: this is the the total power (\f$\Phi\f$) emitted
        uniformly over the sphere.  The incident normal irradiance at
        a point distance \f$r\f$ from the light is \f$ E_{\perp} =
        \frac{\Phi}{4 \pi r^2} \f$.
        
        Spot light: the power is the same as for a point light, but
        line of sight is zero outside the spot cone.  Thus the area
        within the spot cone does not change illumination when the
        cone shrinks.

        Directional light: this is the incident normal irradiance
        in the light's direction, \f$E_\perp\f$.
    */
    Color3              color;

    Light();

protected:
    
    /** Update m_frame's rotation from spotDirection and spotTarget. Called from factory methods
       to support the old API interface.
    */
    void computeFrame(const Vector3& spotDirection, const Vector3& rightDirection);

    void init(const String& name, AnyTableReader& propertyTable);

    /**
        Takes a 2D random vector, samples that point in the projected spherical quad,
        then returns the world-space position on the light of that sample. (Urena, 2013)
        \cite https://www.arnoldrenderer.com/research/egsr2013_spherical_rectangle.pdf
    */
	Vector3 sampleSphericalQuad(const Point3& origin, float u, float v, float& solidAngle) const;

public:

    CullFace shadowCullFace() const {
        return m_shadowCullFace;
    }

    /** \param scene may be nullptr */
    static shared_ptr<Entity> create
       (const String&               name, 
        Scene*                      scene,
        AnyTableReader&             propertyTable, 
        const ModelTable&           modelTable = ModelTable(),
        const Scene::LoadOptions&   options = Scene::LoadOptions());

    /** Is vector w_i (from a point in the scene to the light) within the field of view (e.g., spotlight cone) of this light?  Called from biradiance(). */
    bool inFieldOfView(const Vector3& w_i) const;

    Type type() const {
        return m_type;
    }

    /**
     Assumes type() == AREA and isRectangular = true. Returns the world space position of the vertices.

     Iterates counter-clockwise from the center - extent/2 corner.
     */
    void getRectangularAreaLightVertices(Point3 position[4]) const;

    void setSpotHalfAngle(float rad) {
        m_spotHalfAngle = rad;
    }

    /** Biradiance (radiant flux per area) due to the entire emitter to point X, using the light's specified falloff and spotlight doors.*/
    Biradiance3 biradiance(const Point3& X) const;

    Biradiance3 biradiance(const Point3& X, const Point3& pointOnAreaLight) const;

    bool enabled() const {
        return m_enabled;
    }

    /** Like Photoshop brush hardness. 1.0 = abrupt cutoff at the half angle (default), 0.0 = gradual falloff within the cone.*/
    float spotHardness() const {
        return m_spotHardness;
    }

    void setSpotHardness(float f) {
        m_spotHardness = clamp(f, 0.0f, 1.0f);
    }

    /** Distance in meters to pull point light approximations of area lights back by. */
    float areaLightPullback() const {
        return m_areaLightPullback;
    }

    void setAreaLightPullback(float p) {
        m_areaLightPullback = p;
    }

    /** Returns a number between 0 and 1 for how the light falls off due to the spot light's cone. */
    float spotLightFalloff(const Vector3& w_i) const;

    /** For a SPOT or OMNI light, the power of the bulb.  A SPOT light also has "barn doors" that
        absorb the light leaving in most directions, so their emittedPower() is less. 

        For an AREA light, this is 1/4 the power emitted by the surface, since the area light only
        emits forward (the back half is black, so there's a factor of 1/2) and it emits uniformly
        from a planar surface so has a cosine falloff with angle that integrates to another 1/2
        over the hemisphere.
        
        If a SPOT
        light uses a spotLightFalloff(), then it not simulated correctly for indirect light by
        G3D::PathTracer or the rasterization renderer when looking directly at the light
        because the cosine spotlight falloff isn't taken into account by the assumed uniform
        emission model of the emissive textures.

        This is infinite for directional lights.
        \sa emittedPower */
    Power3 bulbPower() const;

    /** Position of the light's shadow map clipping plane along the light's z-axis.*/
    float nearPlaneZ() const;

    /** Position of the light's shadow map clipping plane along the light's z-axis.*/
    float farPlaneZ() const;

    /** Farthest that the farPlaneZ() is ever allowed to be (part of the Light's specification) */
    float farPlaneZLimit() const;

    /** Closest that the nearPlaneZ() is ever allowed to be (part of the Light's specification) */
    float nearPlaneZLimit() const;

    /**
       For a SPOT or OMNI light, the power leaving the light into the scene.  A SPOT light's
       "barn doors" absorb most of the light. (A real spot light has a reflector at the back so
       that the first half of the emitted light is not also lost, however this model is easier
       to use when specifying scenes.)

       Useful for photon emission.
       This is infinite for directional lights.
      \sa bulbPower
     */
    Power3 emittedPower() const;

    /** \brief Returns a unit vector selected uniformly at random within the world-space 
        solid angle of the emission cone, frustum, or sphere of the light source. For a directional 
        light, simply returns the light direction. */
    Vector3 randomEmissionDirection(class Random& rng) const;

    /** When this light is enabled, does it cast shadows? */
    bool shadowsEnabled() const {
        return m_shadowsEnabled;
    }

    /** Sends directional lights to infinity */
    virtual void setFrame(const CoordinateFrame& c, bool updatePreviousFrame = true) override;

    /** 
        \brief Homogeneous world space position of the center of the light source
        (for a DIRECTIONAL light, w = 0) 

        \sa extent(), frame()
    */
    Vector4 position() const {
        if (m_type == Type::DIRECTIONAL) {
            return Vector4(-m_frame.lookVector(), 0.0f);
        } else {
            return Vector4(m_frame.translation, 1.0f);
        }
    }

    /** Position on the light at coordinates (u, v) in light space, where u and v are each on [-1, 1].
        Does not include the areaLightPullback.
        \sa lowDiscrepancyPosition, randomPosition
    */
    Vector4 position(float u, float v) const {
        return Vector4(m_frame.pointToWorldSpace(Point3(u * m_extent.x * 0.5f, v * m_extent.y * 0.5f, 0.0f)), 1.0f);
    }

    /** Low-discrepancy distributed positions on the light based on screen pixel and light index.
        The sequence is unique for each pixel and lightIndex. It repeats every numSamples.
        Does not include the areaLightPullback.

        \sa randomPosition
      */
    Point3 lowDiscrepancyAreaPosition(int pixelIndex, int lightIndex, int sampleIndex, int numSamples) const;

    Point3 stratifiedAreaPosition(int pixelIndex, int sampleIndex, int numSamples) const;

    Point3 uniformAreaPosition() const;

    /** Low-discrepancy distributed positions on the solid angle subtended by the light
        relative to the samplePosition, based on screen pixel and light index.
        The sequence is unique for each pixel and lightIndex. It repeats every numSamples.
        Does not include the areaLightPullback. Returns probability in w channel.
        \sa randomPosition
    */
    Point3 lowDiscrepancySolidAnglePosition(int pixelIndex, int lightIndex, int sampleIndex, int numSamples, const Point3& X, float& areaTimesPDFValue) const;

    /** Helper function to generate low-discrepancy sample points in 2D. */
    Point2  lowDiscrepancySample(int pixelIndex, int lightIndex, int sampleIndex, int numSamples) const;


    /** Does not include the areaLightPullback.
        \sa lowDiscrepancyPosition, position */
    Vector4 randomPosition(Random& r = Random::threadCommon()) const {
        if (m_type == Type::AREA) {
            Point2 p;
            if (m_rectangular) {
                p = Vector2(r.uniform(-1.0f, 1.0f), r.uniform(-1.0f, 1.0f));
            } else {
                // Rejection sample the disk
                do {
                    p = Vector2(r.uniform(-1.0f, 1.0f), r.uniform(-1.0f, 1.0f));
                } while (p.squaredLength() > 1.0f);
            }
            return position(p.x, p.y);
        } else {
            return position();
        }
    }

    /** Spot light cutoff half-angle in <B>radians</B>.  pi() = no
        cutoff (point/dir).  Values less than pi()/2 = spot light.

        A rectangular spot light circumscribes the cone of this angle.
        That is, spotHalfAngle() is the measure of the angle from 
        the center to each edge along the orthogonal axis.
    */
    float spotHalfAngle() const {
        return m_spotHalfAngle;
    }

    /** \deprecated \sa rectangular() */
    [[deprecated]] bool spotSquare() const {
        return m_rectangular;
    }

    /** Can this light possibly illuminate anything in the box based on the spotlight and attenuation? */
    bool possiblyIlluminates(const class Sphere& sphere) const;

    /** The translation of a DIRECTIONAL light is infinite.  While
        this is often inconvenient, that inconvenience is intended to
        force separate handling of directional sources.

        Use position() to find the homogeneneous position.

        
     */   
    const CoordinateFrame& frame() const {
        return m_frame;
    }

    /** If there is a bulbPowerTrack, then the bulbPower will be overwritten from it during simulation.*/
    virtual void onSimulation(SimTime absoluteTime, SimTime deltaTime) override;

    /** 
        Optional shadow map.  May be nullptr
    */
    const shared_ptr<class ShadowMap>& shadowMap() const {
        return m_shadowMap;
    }
    
    /** Returns the cosine of the spot light's half angle, and the softness 
        constant used in G3D's lighting model. Only useful for spot lights */
    void getSpotConstants(float& cosHalfAngle, float& softnessConstant) const;

    virtual Any toAny(const bool forceAll = false) const override;

    /** Constructs geometry as needed if visible() and no model() is set already. */
    virtual void onPose(Array<shared_ptr<Surface> >& surfaceArray) override;

    /**
    \param toLight will be normalized
    Only allocates the shadow map if \a shadowMapRes is greater than zero and shadowsEnabled is true
    */
    static shared_ptr<Light> directional(const String& name, const Vector3& toLight, const Radiance3& color, bool shadowsEnabled = true, int shadowMapRes = 2048);

    static shared_ptr<Light> point(const String& name, const Point3& pos, const Power3& color, float constAtt = 0.01f, float linAtt = 0, float quadAtt = 1.0f, bool shadowsEnabled = true, int shadowMapRes = 2048);

    /** \param pointDirection Will be normalized.  Points in the
    direction that light propagates.

    \param halfAngleRadians Must be on the range [0, pi()/2]. This
    is the angle from the point direction to the edge of the light
    cone.  I.e., a value of pi() / 4 produces a light with a pi() / 2-degree
    cone of view.
    */
    static shared_ptr<Light> spot
    (const String& name,
        const Point3& pos,
        const Vector3& pointDirection,
        float halfAngleRadians,
        const Color3& color,
        float constAtt = 0.01f,
        float linAtt = 0,
        float quadAtt = 1.0f,
        bool shadowsEnabled = true,
        int shadowMapRes = 2048);

    /** Creates a spot light that looks at a specific point (by calling spot()) */
    static shared_ptr<Light> spotTarget
    (const String& name,
        const Point3& pos,
        const Point3& target,
        float halfAngleRadians,
        const Color3& color,
        float constAtt = 0.01f,
        float linAtt = 0,
        float quadAtt = 1.0f,
        bool  shadowsEnabled = true,
        int  shadowMapRes = 2048) {
        return spot(name, pos, target - pos, halfAngleRadians, color, constAtt, linAtt, quadAtt, shadowsEnabled);
    }

    /** Returns the sphere within which this light has some noticable effect.  May be infinite.
        \param cutoff The value at which the light intensity is considered negligible. */
    class Sphere effectSphere(float cutoff = 30.0f / 255) const;
    
    /** Distance from the point to the light (infinity for DIRECTIONAL lights). */
    float distance(const Point3& p) const {
        if (m_type == Type::DIRECTIONAL) {
            return finf();
        } else {
            return (p - m_frame.translation).length();
        }
    }


    /** 
       The size ("diameter") of the emitter along the x and y axes of its frame().

       AREA and DIRECTIONAL lights emit from the entire surface.  POINT and SPOT lights
       only emit from the center, although they use the extent for radial falloff to 
       avoid superbrightenening. Extent is also used for Draw::light, debugging
       and selection by SceneEditorWindow. 
      
       \cite http://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation
      */
    const Vector2& extent() const {
        return m_extent;
    }

    static int findBrightestLightIndex(const Array<shared_ptr<Light> >& array, const Point3& point);

    static shared_ptr<Light> findBrightestLight(const Array<shared_ptr<Light> >& array, const Point3& point);

        

    /** If true, the emitter (and its emission cone for a spot light)
        is rectangular instead elliptical.

        Defaults to false. */
    bool rectangular() const {
        return m_rectangular;
    }

    /** In a global illumination renderer, should this light create
        indirect illumination (in addition to direct illumination)
        effects (e.g., by emitting photons in a photon mapper)?
        
        Defaults to true.*/
    bool producesIndirectIllumination() const {
        return m_producesIndirectIllumination;
    }

    bool producesDirectIllumination() const {
        return m_producesDirectIllumination;
    }


    /** Sets the following arguments in \param args:
        \code
        vec4  prefix+position;
        vec3  prefix+color;
        vec4  prefix+attenuation;
        vec3  prefix+direction;
        bool  prefix+rectangular;
        vec3  prefix+up;
        vec3  prefix+right;
        float prefix+radius;
        prefix+shadowMap...[See ShadowMap::setShaderArgs]
        \endcode
        */
    void setShaderArgs(class UniformTable& args, const String& prefix) const;

    void setShadowsEnabled(bool shadowsEnabled) {
        if (m_shadowsEnabled != shadowsEnabled) {
            m_shadowsEnabled = shadowsEnabled;
            m_lastChangeTime = System::time();
        }
    }    

    void setEnabled(bool enabled) {
        if (m_enabled != enabled) {
            m_enabled = enabled;
            m_lastChangeTime = System::time();
        }  
    }    

    virtual void makeGUI(class GuiPane* pane, class GApp* app) override;

    /** Update the shadow maps in the enabled shadow casting lights from the array of surfaces.
    
      \param cullFace If CullFace::CURRENT, the Light::shadowCullFace is used for each light.*/
    static void renderShadowMaps(RenderDevice* rd, const Array<shared_ptr<Light> >& lightArray, const Array<shared_ptr<Surface> >& allSurfaces, CullFace cullFace = CullFace::CURRENT);
};

} // namespace

G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::Light::Type);

