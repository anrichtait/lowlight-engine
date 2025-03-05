/**
  \file test/tSystemMemcpy.cpp

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



void perfSystemMemcpy() {
    PRINT_SECTION("Performance: System::memcpy", "Checks performance of memory copy against standard library");

    // Number of memory sizes to test
    static const int M = 8;

    //  Repeats per memory size
    static const int trials = 300;

    static const int SIZES[M] = { 1 * 1024, 16 * 1024, 128 * 1024, 256 * 1024, 768 * 1024, 1024 * 1024, 2048 * 1024, 4096 * 1024 };

    chrono::nanoseconds native[M], g3d[M];

    Stopwatch stopwatch;
    for (int m = 0; m < M; ++m) {
        const int n = SIZES[m];
        void* m1 = System::alignedMalloc(n, 16);
        void* m2 = System::alignedMalloc(n, 16);

        testAssertM((((intptr_t)m1) % 16) == 0, "Memory is not aligned correctly");
        testAssertM((((intptr_t)m2) % 16) == 0, "Memory is not aligned correctly");

        // First iteration just primes the system
        ::memcpy(m1, m2, n);
        stopwatch.tick();
            for (int j = 0; j < trials; ++j) {
                ::memcpy(m1, m2, n);
            }
        stopwatch.tock();
        native[m] = stopwatch.elapsedDuration();

        System::memcpy(m1, m2, n);
        stopwatch.tick();
            for (int j = 0; j < trials; ++j) {
                System::memcpy(m1, m2, n);
            }
        stopwatch.tock();
        g3d[m] = stopwatch.elapsedDuration();

        System::alignedFree(m1);
        System::alignedFree(m2);
    }

    std::string labels[M];
    std::string results[M];
    for (int i = 0; i < M; ++i) {
        labels[i] = std::to_string(SIZES[i] / 1024) + "KB";
        results[i] = (g3d[i] < native[i] * 1.1) ? "ok" : "FAIL";
        native[i] = native[i] / trials / 1024;
        g3d[i] = g3d[i] / trials / 1024;
    }

    PRINT_TEXT("", labels);
    PRINT_MICRO("::memcpy", "(us/KB)", native);
    PRINT_MICRO("System::memcpy", "(us/KB)", g3d);
    PRINT_TEXT("Outcome", results);
}


void testSystemMemcpy() {
    printf("System::memcpy ");
	static const int k = 50000;
	static uint8 a[k];
	static uint8 b[k];

	int i;
	
	for (i = 0; i < k; ++i) {
		a[i] = i & 255;
	}

	System::memcpy(b, a, k);

	for (i = 0; i < k; ++i) {
		testAssert(b[i] == (i & 255));
		testAssert(a[i] == (i & 255));
	}
    printf("passed\n");

}

