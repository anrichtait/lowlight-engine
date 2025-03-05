/**
  \file G3D-app.lib/include/G3D-app/DepthOfFieldSettings.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#ifndef GLG3D_DepthOfFieldSettings_h
#define GLG3D_DepthOfFieldSettings_h

#include "G3D-base/platform.h"
#include "G3D-base/enumclass.h"
#include "G3D-app/GBuffer.h"

namespace G3D {

class Any;

/**
The depth of field model.

\sa DepthOfFieldSettings, DepthOfField
*/
class DepthOfFieldModel {
public:

    enum Value {
        /** Pinhole lens, circle of confusion is always zero.*/
        NONE,

        /** In this model, the circle of confusion is determined by
            the Gaussian lens model for an ideal single-lens
            camera. */
        PHYSICAL, 

        /** In this model, the circle of confusion is determined by
            linear interpolation between depth stops in an explicit
            gradient:
          

            <pre>
                eye     nearBlurryPlaneZ   nearSharpPlaneZ    farSharpPlaneZ   farBlurryPlaneZ

                &lt;)             |                 .                 .                 |
                                |                                                     |
            </pre>
        */
        ARTIST
    };
    
private:

    static const char* toString(int i, Value& v) {
        static const char* str[] = {"NONE", "PHYSICAL", "ARTIST", nullptr}; 
        static const Value val[] = {NONE, PHYSICAL, ARTIST};
        const char* s = str[i];
        if (s) {
            v = val[i];
        }
        return s;
    }

    Value value;

public:

    G3D_DECLARE_ENUM_CLASS_METHODS(DepthOfFieldModel);

    operator Value() const {
        return value;
    }
};


} // namespace G3D

G3D_DECLARE_ENUM_CLASS_HASHCODE(G3D::DepthOfFieldModel);



namespace G3D {

class DepthOfFieldSettings {
private:
    
    bool                        m_enabled;

    /** Aperture in meters.  Used for DOF.  Does not affect intensity. */
    float                       m_lensRadius;

    /** Negative number */
    float                       m_focusPlaneZ;

    DepthOfFieldModel           m_model;
    
    /** Maximum defocus blur in the near field, as a fraction of the
        screen size along the axis indicated by the field of view. */
    float                       m_nearRadiusFraction;

    /** Z-plane at which the m_nearRadiusFraction blur is reached.
        This must be greater (less negative) than nearSharpZ */
    float                       m_nearBlurryZ;

    /** Z-plane at which the in-focus field begins.  This must be
        greater (less negative) than farSharpZ */
    float                       m_nearSharpZ;

    /** Z-plane at which the in-focus ends begins.  This must be
        greater (less negative) than farBlurryZ */
    float                       m_farSharpZ;

    /** Z-plane at which the m_farRadiusFraction blur is reached.  This must be
        less (more negative) than farSharpZ */
    float                       m_farBlurryZ;

    /** Maximum defocus blur in the near field, as a fraction of the
        screen size along the axis indicated by the field of view. */
    float                       m_farRadiusFraction;

    /** Divide the image size by this in both directions to speed processing
        for large blurs at the cost of some flickering and boxy artifacts for small blurs. 1,2,3,4 are common values */
    int                         m_reducedResolutionFactor = 2;

    /** This is not persisted. */
    bool                        m_diskFramebuffer = false;

public:

    DepthOfFieldSettings();

    DepthOfFieldSettings(const Any&);

    Any toAny() const;

    void setDiskFramebuffer(bool b) {
        m_diskFramebuffer = b;
    }

    bool diskFramebuffer() const {
        return m_diskFramebuffer;
    }

    bool enabled() const {
        return m_enabled;
    }

    void setEnabled(bool e) {
        m_enabled = e;
    }

    void setModel(DepthOfFieldModel m) {
        m_model = m;
    }
    
    DepthOfFieldModel model() const {
        return m_model;
    }

    void setModel(DepthOfFieldModel::Value m) {
        m_model = m;
    }

    DepthOfFieldModel::Value modelValue() const {
        return (DepthOfFieldModel::Value)m_model;
    }

    /** Divide the image size by this in both directions to speed processing
        for large blurs at the cost of some flickering and boxy artifacts for small blurs. 1,2,3,4 are common values */
    void setReducedResolutionFactor(int f) {
        m_reducedResolutionFactor = f;
    }

    int reducedResolutionFactor() const {
        return m_reducedResolutionFactor;
    }

    /** Maximum defocus blur in the near field under the ARTIST depth
        of field model, as a fraction of the screen size along the
        axis indicated by the field of view.  */
    void setNearBlurRadiusFraction(float r) { 
        m_nearRadiusFraction = r;
    }

    float nearBlurRadiusFraction() const {
        return m_nearRadiusFraction;
    }

    /** Set the plane at which the maximum blur radius is reached in
        the near field under the ARTIST depth of field model.
        Adjusts the other plane depths to maintain a legal model.
    */
    void setNearBlurryPlaneZ(float z) {
        m_nearBlurryZ = z;
        m_nearSharpZ  = min(m_nearBlurryZ - 0.001f, m_nearSharpZ);
        m_farSharpZ   = min(m_nearSharpZ - 0.001f, m_farSharpZ);
        m_farBlurryZ  = min(m_farSharpZ - 0.001f, m_farBlurryZ);
    }

    float nearBlurryPlaneZ() const {
        return m_nearBlurryZ;
    }

    void setNearSharpPlaneZ(float z) {
        m_nearSharpZ = z;
        m_nearBlurryZ = max(m_nearBlurryZ, 0.001f + m_nearSharpZ);
        m_farSharpZ   = min(m_nearSharpZ - 0.001f, m_farSharpZ);
        m_farBlurryZ  = min(m_farSharpZ - 0.001f, m_farBlurryZ);
    }

    float nearSharpPlaneZ() const {
        return m_nearSharpZ;
    }

    void setFarSharpPlaneZ(float z) {
        m_farSharpZ = z;
        m_farBlurryZ  = min(m_farSharpZ - 0.001f, m_farBlurryZ);
        m_nearSharpZ  = max(m_farSharpZ + 0.001f, m_nearSharpZ);
        m_nearBlurryZ = max(m_nearBlurryZ, 0.001f + m_nearSharpZ);
    }

    float farSharpPlaneZ() const {
        return m_farSharpZ;
    }

    void setFarBlurryPlaneZ(float z) {
        m_farBlurryZ = z;
        m_farSharpZ   = max(m_farSharpZ, 0.001f + m_farBlurryZ);
        m_nearSharpZ  = max(m_farSharpZ + 0.001f, m_nearSharpZ);
        m_nearBlurryZ = max(m_nearBlurryZ, 0.001f + m_nearSharpZ);
    }

    float farBlurryPlaneZ() const {
        return m_farBlurryZ;
    }

    /** Maximum defocus blur in the near field under the ARTIST depth
        of field model, as a fraction of the screen size along the
        axis indicated by the field of view.  */
    void setFarBlurRadiusFraction(float r) { 
        m_farRadiusFraction = r;
    }

    float farBlurRadiusFraction() const {
        return m_farRadiusFraction;
    }

    /** Plane that is in focus under a lens camera (PHYSICAL depth of
        field model). This is a negative number unless you intend to focus behind the camera. */
    void setFocusPlaneZ(float z) {
        m_focusPlaneZ = z;
    }
    
    float focusPlaneZ() const {
        return m_focusPlaneZ;
    }

    /** Radius of the lens in meters under the PHYSICAL depth of field model. */
    void setLensRadius(float r) {
        m_lensRadius = r;
    }

    float lensRadius() const {
        return m_lensRadius;
    }

    /**
     \param z In camera space; should be NEGATIVE
     \param edgeToEdgeFieldOfView Angular field of view along the X or Y axis, corresponding to the screenPixelSize axis
     \param screenPixelSize Width or height of the screen, in pixels (choose one based on the field of view direction)

     \return Signed circle of confusion radius in pixels

     If negative, then z is closer to the camera than the focus depth.
     If positive, then z is farther.

     \sa Camera::circleOfConfusionRadiusPixels, Camera::maxCircleOfConfusionRadiusPixels
     */
    float circleOfConfusionRadiusPixels(const float z, const float edgeToEdgeFieldOfView, const float screenPixelSize) const;

    /**
    Ensures the GBuffer specification has all the fields needed to render this effect
    \sa GApp::extendGBufferSpecification
    */
    void extendGBufferSpecification(GBuffer::Specification& spec) const;

};

}

#endif
