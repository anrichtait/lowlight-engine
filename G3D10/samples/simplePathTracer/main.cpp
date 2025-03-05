/************* Simple Path Tracer by Morgan McGuire @CasualEffects, July 2019 ****************/

// You can download G3D from https://casual-effects.com/g3d for Windows, Mac, and Linux
#include <G3D/G3D.h>

/* A "pure" path tracer with importance sampling and naive multithreading.

   Path exploration is terminated via Russian Roulette. There is no direct illumination 
   (a.k.a. next event estimation). Instead, the algorithm assumes that "lights" are 
   emissive area sources that will be found randomly. 

   Uses the following helpers:

   - G3D::Scene for scene data file parsing
   - G3D::TriTree for ray-triangle intersection and texture map sampling
   - G3D::Surfel for BSDF importance sampling
   - G3D::Image::forEachPixel for brute-force multithreading
   - G3D::Camera for generating world-space primary rays

   This is about as simple of an implementation as possible that gives usable performance 
   for real scenes. It renders Sponza at 128x96 with 512 paths per pixel in about 10 seconds. 
   G3D::PathTracer achieves about 10x higher throughput on a CPU using significantly more 
   complex optimizations. Another factor of 20x is achievable by moving the entire computation
   onto a ray tracing GPU with G3D::OptiXTriTree. */
class PTApp : public GApp {
public:
    const int           pathsPerPixel      = 1024;

    /* Small distance to adjust recursive rays to avoid intersecting the same surface */
    const float         epsilon            = 1e-6f * units::meters();

    shared_ptr<TriTree> tree;
    
    /* Trace this (world space) ray and return the radiance it encounters */
    Radiance3 L_i(const Ray& ray) const {
        // The light in at Y from direction w is equal to the light
        // out at some other point X in the opposite direction if
        // there is a homogeneous medium between them: L_i(Y, w) =
        // L_o(X, -w). Find this point X and let the outgoing
        // direction w_o at X be -w.
        const shared_ptr<Surfel>& surfel = tree->intersectRay(ray);

        // Outgoing light direction, from which the path arrived
        const Vector3& w_o = -ray.direction();

        // Light emitted by the surface
        const Radiance3& L_e = surfel->emittedRadiance(w_o);
        
        // Incoming light direction, to be determined by scatter()
        Vector3 w_i;
        
        // G3D::Surfel::scatter assigns to the weight the product of the projected area * BSDF,
        // Monte Carlo integration weight, and [Monte Carlo] Russian Roulette factors:
        //
        //  weight = (rnd() <= rrProb(f)) ? (f(w_i, w_o) * |w_i.dot(n)|) / (monteCarloDistribution(w_i) * rrProb(f)) : 0
        //
        // The rrProb() and monteCarloDistribution() can be any functions so long as they are nonzero at the right
        // locations; G3D::UniversalSurfel::scatter chooses them to be close to numerically optimal for minimizing
        // the number of samples needed for path tracing when incoming light is uniform with respect to direction.
        Color3 weight;
        
        // Computes the weight and direction w_i
        if (surfel->scatter(PathDirection::EYE_TO_SOURCE, w_o, true, Random::threadCommon(), weight, w_i)) {
            // Intersection point on the surface
            const Point3& X = surfel->position;
            
            // Cast the recursive ray slightly offset from the hit point along both the normal and ray direction
            const Ray& nextRay = Ray(X, w_i, epsilon, finf()).bumpedRay(epsilon, surfel->geometricNormal);
            
            // Evaluate the recursive ray. This is the single-sample MC integrator of the Rendering Equation.
            return L_e + L_i(nextRay) * weight;
        } else {
            return L_e;
        }
    }
    
    /* Trace the whole image */
    void traceImage() {
        const shared_ptr<Image>& image = Image::create(512, 256, ImageFormat::RGB32F());
        tree = TriTree::create(scene(), ImageStorage::COPY_TO_CPU);
        const shared_ptr<Camera>& camera = activeCamera();

        Stopwatch timer; timer.tick();

        // Monte Carlo estimation across the pixel with many samples
        image->forEachPixel<Radiance3>([&](Point2int32 pixel) {
                Random& rng = Random::threadCommon();
                Radiance3 sum;
                // Compute the average radiance into this pixel
                for (int p = 0; p < pathsPerPixel; ++p) {
                    sum += 30.0f * L_i(camera->worldRay(pixel.x + rng.uniform(), pixel.y + rng.uniform(), image->bounds()));
                }
                return sum / float(pathsPerPixel);
            });
        
        timer.tock(); debugPrintf("Elapsed time: %4.1fs\n", timer.elapsedTime());

        // Apply gamma correction, bloom, and post-process antialiasing, and then display
#       ifndef G3D_OSX
            // broken on MacOS Intel drivers :(
            show(m_film->exposeAndRender(renderDevice, camera->filmSettings(), image)); 
#       else
            show(image);
#       endif
    }

    virtual void onInit() override {
        GApp::onInit();
        developerWindow->cameraControlWindow->setVisible(false);
        showRenderingStats = false;
        //loadScene("G3D Simple Cornell Box (Area Light)");
        loadScene("G3D Sponza (Area Light)");
        traceImage();
    }
};


G3D_START_AT_MAIN();
int main(int argc, const char* argv[]) {
    return PTApp().run();
}
