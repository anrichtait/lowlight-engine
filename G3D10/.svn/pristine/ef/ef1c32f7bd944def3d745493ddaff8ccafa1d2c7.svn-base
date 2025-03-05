#ifndef common_glsl
#define common_glsl

#include <g3dmath.glsl>
#include <noise.glsl>

uniform vec2 iMouse;
uniform vec2 iMouseVelocity;

const bool autoRotate = true;
const bool debugCloudsMode = false;

struct Material { Color3 color; float metal; float smoothness; };
struct Surfel { Point3 position; Vector3 normal; Material material; };
struct Sphere { Point3 center; float radius; Material material; };

   
/** Analytic ray-sphere intersection. */
bool intersectSphere(Point3 C, float r, Ray R, inout float nearDistance, inout float farDistance) { Point3 P = R.origin; Vector3 w = R.direction; Vector3 v = P - C; float b = 2.0 * dot(w, v); float c = dot(v, v) - square(r); float d = square(b) - 4.0 * c; if (d < 0.0) { return false; } float dsqrt = sqrt(d); float t0 = infIfNegative((-b - dsqrt) * 0.5); float t1 = infIfNegative((-b + dsqrt) * 0.5); nearDistance = min(t0, t1); farDistance  = max(t0, t1); return (nearDistance < inf); }

const float       verticalFieldOfView = 25.0 * pi / 180.0;

// Directional light source
const Vector3     w_i             = Vector3(1.0, 1.3, 0.6) / 1.7464;
const Biradiance3 B_i             = Biradiance3(2.7);

const Point3      planetCenter    = Point3(0);

// Including clouds
const float       planetMaxRadius = 1.0;
const float       cloudMinRadius  = 0.85;

const Radiance3   atmosphereColor = Color3(0.3, 0.6, 1.0) * 1.0;

vec2 iResolution = g3d_FragCoordExtent.xy;

// This can return negative values, in order to make derivatives smooth. Always
// clamp before using as a density. 
float cloudDensity(Point3 X) {
	return (noise(X * 4.5 - g3d_SceneTime * 0.07, 3) - 0.45);
}
#endif
