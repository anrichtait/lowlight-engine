/**
  \file test/tCollisionDetection.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "testassert.h"
#include "printhelpers.h"

using G3D::uint8;
using G3D::uint32;
using G3D::uint64;

static void measureTriangleCollisionPerformance() {
    Stopwatch stopwatch;
    {
    chrono::nanoseconds raw, opt;

    Vector3 v0(0, 0, 0);
    Vector3 v1(0, 0, -1);
    Vector3 v2(-1, 0, 0);
    Sphere sphere(Vector3(.5,1,-.5), 1);
    Vector3 vel(0, -1, 0);
    Vector3 location;
    float normal[3];
    Triangle triangle(v0, v1, v2);
    int n = 1024;
    int i;

    stopwatch.tick();
    for (i = 0; i < n; ++i) {
        float t = CollisionDetection::collisionTimeForMovingSphereFixedTriangle(sphere, vel, Triangle(v0, v1, v2), location, normal);
        (void)t;
    }
    stopwatch.tock();
    raw = stopwatch.elapsedDuration();

    stopwatch.tick();
    for (i = 0; i < n; ++i) {
        float t = CollisionDetection::collisionTimeForMovingSphereFixedTriangle(sphere, vel, triangle, location, normal);
        (void)t;
    }
    stopwatch.tock();
    opt = stopwatch.elapsedDuration();

    PRINT_HEADER("Sphere-Triangle collision detection");
    PRINT_MICRO("3 vertices", "(us)", raw / n);
    PRINT_MICRO("Triangle", "(us)", opt / n);
    }
    {
        chrono::nanoseconds raw, CD;

        Vector3 v0(0, 0, 0);
        Vector3 v1(0, 0, -1);
        Vector3 v2(-1, 0, 0);
        Sphere sphere(Vector3(0.5f, 1.0f, -0.5f), 1);
        Vector3 vel(0, -1, 0);
        Vector3 location, normal;
        Triangle triangle(v0, v1, v2);
		Vector3 start(3.0f, -1.0f, -0.25f);
        int n = 1024;
        int i;
        Ray ray = Ray::fromOriginAndDirection(start, vel);

        stopwatch.tick();
        for (i = 0; i < n; ++i) {
            double t = ray.intersectionTime(triangle);
            (void)t;
        }
        stopwatch.tock();
        raw = stopwatch.elapsedDuration();
        stopwatch.tick();
        for (i = 0; i < n; ++i) {
            double t = CollisionDetection::collisionTimeForMovingPointFixedTriangle(
                start, vel, triangle, location, normal);
            (void)t;
        }
        stopwatch.tock();
        CD = stopwatch.elapsedDuration();

        PRINT_HEADER("ray.intersectionTime(triangle)");
        PRINT_MICRO("Miss", "(us)", raw / n);

        PRINT_HEADER("collisionTimeForMovingPointFixedTriangle");
        PRINT_MICRO("Miss", "(us)", CD / n);
    }
    {
        chrono::nanoseconds raw ;

        Vector3 v0(0, 0, 0);
        Vector3 v1(0, 0, -1);
        Vector3 v2(-1, 0, 0);
        Vector3 vel(0, -1, 0);
        Vector3 location, normal;
		Vector3 start(-0.15f, 1.0f, -0.15f);
        Triangle triangle(v0, v1, v2);
        int n = 1024;
        int i;
        Ray ray = Ray::fromOriginAndDirection(start, vel);

        stopwatch.tick();
        for (i = 0; i < n; ++i) {
            double t = ray.intersectionTime(triangle);
            (void)t;
        }
        stopwatch.tock();
        raw = stopwatch.elapsedDuration();

        PRINT_HEADER("ray.intersectionTime(triangle)");
        PRINT_MICRO("Hit", "(us)", raw / n);
    }
}
 

static void measureAABoxCollisionPerformance() {
    chrono::nanoseconds raw, opt;

    AABox aabox(Vector3(-1, -1, -1), Vector3(1,2,3));
    Box   box = aabox;

    Vector3 pt1(0,10,0);
    Vector3 vel1(0,-1,0);
    Vector3 location, normal;
    int n = 1024;
    int i;

    Stopwatch stopwatch;
    stopwatch.tick();
    for (i = 0; i < n; ++i) {
        double t = CollisionDetection::collisionTimeForMovingPointFixedBox(
            pt1, vel1, box, location, normal);
        (void)t;
    }
    stopwatch.tock();
    raw = stopwatch.elapsedDuration();

    stopwatch.tick();
    for (i = 0; i < n; ++i) {
        double t = CollisionDetection::collisionTimeForMovingPointFixedAABox(
            pt1, vel1, aabox, location);
        (void)t;
    }
    stopwatch.tock();
    opt = stopwatch.elapsedDuration();
    
    PRINT_HEADER("AABox Collisions");
    PRINT_MICRO("Ray-Box", "(us)", raw / n);
    PRINT_MICRO("Ray-AABox", "(us)", opt / n);
}


void testCollisionDetection() {
    printf("CollisionDetection ");

    {
        Vector3 pos(2.0f, 0.0f, 2.5f); 
        Vector3 dir = Vector3(-1.0f, 0.0f, -1.0f).direction(); 

        Box myBox(G3D::Vector3(-1,-1,-1), G3D::Vector3(1,1,1)); 
        Vector3 loc, normal;

        float t = CollisionDetection::collisionTimeForMovingPointFixedBox(pos, dir, myBox, loc, normal);
        (void)t;
        testAssert(normal.fuzzyEq(Vector3(0,0,1)));
    }


    {
        Sphere s(Vector3(0,1,0), 1);
        Plane  p = Plane::fromEquation(0,1,0,0);
        Array<Vector3> contact;
        Array<Vector3> outNormal;

        float depth = CollisionDetection::penetrationDepthForFixedSphereFixedPlane(
            s, p, contact, outNormal);

        testAssert(outNormal[0] == p.normal());
        testAssert(contact[0] == Vector3::zero());
        testAssert(depth == 0);

        s = Sphere(Vector3(0,2,0), 1);

        depth = CollisionDetection::penetrationDepthForFixedSphereFixedPlane(
            s, p, contact, outNormal);

        testAssert(depth < 0);
    }

    {
        Sphere s(Vector3(0,1,0), 1);
        Sphere r(Vector3(0,-.5,0), 1);
        Array<Vector3> contact;
        Array<Vector3> outNormal;

        float depth = CollisionDetection::penetrationDepthForFixedSphereFixedSphere(
            s, r, contact, outNormal);
        (void)depth;

        testAssert(outNormal[0] == Vector3(0,-1,0));
        testAssert(depth == .5);
    }

    {
        Sphere s(Vector3(5, 0, 0), 1);
        Sphere f(Vector3(4.5, 0, 0), 1);
        testAssert(CollisionDetection::fixedSolidSphereIntersectsFixedSolidSphere(s, f));
    }

    {
        Sphere s(Vector3(5, 0, 0), 1);
        Sphere f(Vector3(1, 0, 0), 2);
        testAssert(! CollisionDetection::fixedSolidSphereIntersectsFixedSolidSphere(s, f));
    }

    {
        Sphere s(Vector3(5, 0, 0), 1);
        Box    f(Vector3(-1, -1.5f, -2), Vector3(4.5f, 2, 8));
        testAssert(CollisionDetection::fixedSolidSphereIntersectsFixedSolidBox(s, f));
    }

    {
        Sphere s(Vector3(0, 0, 0), .1f);
        Box    f(Vector3(-1, -1.5f, -2), Vector3(4.5f, 2, 8));
        testAssert(CollisionDetection::fixedSolidSphereIntersectsFixedSolidBox(s, f));
    }

    {
        Sphere s(Vector3(6, 0, 0), 1);
        Box    f(Vector3(-1, -1.5f, -2), Vector3(4.5f, 2, 8));
        testAssert(! CollisionDetection::fixedSolidSphereIntersectsFixedSolidBox(s, f));
    }

    {
        Vector3 v0(0, 0, 0);
        Vector3 v1(0, 0, -1);
        Vector3 v2(-1, 0, 0);
        Ray ray = Ray::fromOriginAndDirection(Vector3(-.15f, 1,-.15f), Vector3(0, -1, 0));
        Vector3 location, normal;
        double t = ray.intersectionTime(v0, v1, v2);
        testAssert(t == 1.0);

		ray = Ray::fromOriginAndDirection(ray.origin() * Vector3(1,-1,1), ray.direction());
        t = ray.intersectionTime(v0, v1, v2);
        testAssert(t == inf());

        // One-sided test
		ray = Ray::fromOriginAndDirection(ray.origin() * Vector3(1,-1,1), ray.direction() * Vector3(1,-1,1));
        t = ray.intersectionTime(v0, v1, v2);
        testAssert(t == inf());

        // Time scale
        ray = Ray::fromOriginAndDirection(Vector3(-.15f, 1,-.15f), Vector3(0, -1, 0));
        t = ray.intersectionTime(v0, v1, v2);

        testAssert(t == 1.0);
    }

    {
        Vector3 v0(0, 0, 0);
        Vector3 v1(0, 0, -1);
        Vector3 v2(-1, 0, 0);
        Ray ray = Ray::fromOriginAndDirection(Vector3(-0.15f, 1,-0.15f), Vector3(0, -1, 0));
        Vector3 location, normal;
        float t = CollisionDetection::collisionTimeForMovingPointFixedTriangle(ray.origin(), ray.direction(), v0, v1, v2);
        testAssert(t == 1.0);
        
        t = CollisionDetection::collisionTimeForMovingPointFixedTriangle(ray.origin(), ray.direction(), v0, v1, v2, location, normal);
        testAssert(t == 1.0);
        testAssert(location.fuzzyEq(ray.origin() + ray.direction() * t));
        testAssert(normal.fuzzyEq(Vector3::unitY()));

		ray = Ray::fromOriginAndDirection(ray.origin() * Vector3(1,-1,1), ray.direction());
        t = CollisionDetection::collisionTimeForMovingPointFixedTriangle(ray.origin(), ray.direction(), v0, v1, v2);
        testAssert(t == inf());

        // One-sided test
		ray = Ray::fromOriginAndDirection(ray.origin() * Vector3(1,-1,1), ray.direction() * Vector3(1,-1,1));
        t = CollisionDetection::collisionTimeForMovingPointFixedTriangle(ray.origin(), ray.direction(), v0, v1, v2);
        testAssert(t == inf());


        // Time scale
        ray = Ray::fromOriginAndDirection(Vector3(-.15f, 1,-.15f), Vector3(0, -1, 0));
        t = CollisionDetection::collisionTimeForMovingPointFixedTriangle(ray.origin(), ray.direction(), v0, v1, v2);

        testAssert(t == 1.0);
    }


    {
        Sphere s = Sphere(Vector3(0, 1, 0), 1.5);
        
        Vector3 v = -Vector3::unitY();
        
        //  .:::.
        // ::::::: 
        //::::X::*:
        // ::::::| \             .
        //  ':::'*--*
        //
        
        Triangle tri = Triangle(Vector3(1, 0, 0),
                                Vector3(1, 1, 0),
                                Vector3(2, 0, 0));
        Vector3 outLocation;
        
        float time = CollisionDetection::collisionTimeForMovingSphereFixedTriangle(s, v, tri, outLocation);
        
        testAssertM(time == 0.0f, "Missed pre-existing collision");
        testAssertM(outLocation.fuzzyEq(Vector3(1,1,0)), "Wrong collision location");
    }

    printf("passed\n");
}


void perfCollisionDetection() {
    PRINT_SECTION("Performance: Collision Detection", "");
	measureTriangleCollisionPerformance();
	measureAABoxCollisionPerformance();
}
