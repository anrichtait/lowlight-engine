/**
  \file test/tTextOutput.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "printhelpers.h"
#include "testassert.h"

using G3D::uint8;
using G3D::uint32;
using G3D::uint64;

void perfTextOutput() {
    PRINT_SECTION("Performance: TextOutput", "");

    // printf
    {
        TextOutput t;
        char buf[2048];

        Stopwatch stopwatch;
        chrono::nanoseconds tt, tf, tp;
        const int N = 5000;

        stopwatch.tick();
        for (int i = 0; i < N; ++i){
            sprintf(buf, "%d, %d, %d\n", i, i + 1, i + 2);
        }
        stopwatch.tock();
        tp = stopwatch.elapsedDuration();

        String s;
        stopwatch.tick();
        for (int i = 0; i < N; ++i){
            s = format("%d, %d, %d\n", i, i + 1, i + 2);
        }
        stopwatch.tock();
        tf = stopwatch.elapsedDuration();

        stopwatch.tick();
        for (int i = 0; i < N; ++i){
            t.printf("%d, %d, %d\n", i, i + 1, i + 2);
        }
        stopwatch.tock();
        tt = stopwatch.elapsedDuration();
        t.commitString(s);

        int k = 3;
        PRINT_HEADER("Printing int32");
        PRINT_MICRO("sprintf", "(us)", tp / (k * N));
        PRINT_MICRO("format", "(us)", tf / (k * N));
        PRINT_MICRO("TextOutput::printf", "(us)", tt / (k * N));
    }
}


