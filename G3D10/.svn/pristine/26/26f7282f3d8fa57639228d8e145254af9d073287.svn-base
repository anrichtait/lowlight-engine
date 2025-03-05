#pragma once
#include <G3D/G3D.h>

/** These match the filenames from the source data from CC0 textures.

File naming convention from CC0 Textures library is:

PNG/
  <texture>/
    2K/
      <texture>_2K_<FieldName>.png

Where FieldName is one of: AmbientOcclusion, Color, Displacement, Normal, Roughness. 
The code will attempt to match other common naming conventions.


See App::loadSource() for construction.
*/
struct Source {

    String              name;

    /** Multiplies the environment. 0 = occluded, 1 = unoccluded (default) */
    shared_ptr<Texture> ambientOcclusion;

    /** 0 = low (default), 1 = high */
    shared_ptr<Texture> displacement;

    /** Read as: 
    
        normalize(texelValue * vec3(2, -2, 2) - vec3(1, -1, 1))
        
        X = right
        Y = up (was down in the original file, flipped by the code here)
        Z = out of the surface, towards the viewer        
        
     */
    shared_ptr<Texture> normal;

    /** Diffuse and glossy color for metal, diffuse for dielectric (which has white glossy) */
    shared_ptr<Texture> color;

    /** 0 = smooth (default), 1 = rough */
    shared_ptr<Texture> roughness;

    /** 0 = dielectric (default), 1 = metal */
    shared_ptr<Texture> metalness;

    /** 0 = matte (default), 1 = glossy */
    shared_ptr<Texture> glossiness;

    Source() {}

    /** The filenameBase should be the complete common path to the texture files, e.g.,
        "data/ChristmasTreeOrnament007/2K/".

        Any missing files will be synthesized. */
    Source(const String& directory);
};


/** \brief Application framework. */
class App : public GApp {
protected:
    shared_ptr<Framebuffer>       m_destination;

    Array<shared_ptr<Texture>>    environmentMapArray;

    void makeGUI();
    void processOneMaterial(const String& materialName) const;
    void render(const Source& source, const Vector3& directionToLight, const Radiance3& lightRadiance, const shared_ptr<Texture>& environmentMap, const shared_ptr<Framebuffer>& destination) const;

public:

    App(const GApp::Settings& settings = GApp::Settings());

    virtual void onInit() override;
    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;
};
