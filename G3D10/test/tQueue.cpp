/**
  \file test/tQueue.cpp

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
#include <deque>

class BigE {
public:
    int x;
    /** Make this structure big */
    int dummy[100];

    BigE() {
        x = 7;
        for (int i = 0; i < 100; ++i) {
            dummy[i] = i;
        }
    }

    ~BigE() {
    }

    BigE(const BigE& a) : x(a.x) {
        for (int i = 0; i < 100; ++i) {
            dummy[i] = a.dummy[i];
        }
    }

    BigE& operator=(const BigE& a) {
        x = a.x;
        for (int i = 0; i < 100; ++i) {
            dummy[i] = a.dummy[i];
        }
        return *this;
    }
};


void perfQueue() {
    PRINT_SECTION("Performance: Queue", "");
    Stopwatch stopwatch;

    chrono::nanoseconds g3dStreamSmall, g3dEnquequeFSmall, g3dEnquequeBSmall;
    chrono::nanoseconds stdStreamSmall, stdEnquequeFSmall, stdEnquequeBSmall;

    chrono::nanoseconds g3dStreamLarge, g3dEnquequeFLarge, g3dEnquequeBLarge;
    chrono::nanoseconds stdStreamLarge, stdEnquequeFLarge, stdEnquequeBLarge;

    int iterations = 1000000;
    int enqueuesize = 10000;
 
    // Number of elements in the queue at the beginning for streaming tests
    int qsize = 1000;
    {
        Queue<int>      g3dQ;
        std::deque<int> stdQ;


        for (int i = 0; i < qsize; ++i) {
            g3dQ.pushBack(i);
            stdQ.push_back(i);
        }

        // Run many times to filter out startup behavior
        for (int j = 0; j < 3; ++j) {

            stopwatch.tick();
            {
                for (int i = 0; i < iterations; ++i) {
                    g3dQ.pushBack(g3dQ.popFront());
                }
            }
            stopwatch.tock();
            g3dStreamSmall = stopwatch.elapsedDuration();

            stopwatch.tick();
            {
                for (int i = 0; i < iterations; ++i) {
                    int v = stdQ[0];
                    stdQ.pop_front();
                    stdQ.push_back(v);
                }
            }
            stopwatch.tock();
            stdStreamSmall = stopwatch.elapsedDuration();
        }
    }
    {
        Queue<int>      g3dQ;
        stopwatch.tick();
        {
            for (int i = 0; i < enqueuesize; ++i) {
                g3dQ.pushFront(i);
            }
        }
        stopwatch.tock();
        g3dEnquequeFSmall = stopwatch.elapsedDuration();

        std::deque<int> stdQ;
        stopwatch.tick();
        {
            for (int i = 0; i < enqueuesize; ++i) {
                stdQ.push_front(i);
            }
        }
        stopwatch.tock();
        stdEnquequeFSmall = stopwatch.elapsedDuration();
    }
    {
        Queue<int>      g3dQ;
        stopwatch.tick();
        {
            for (int i = 0; i < enqueuesize; ++i) {
                g3dQ.pushBack(i);
            }
        }
        stopwatch.tock();
        g3dEnquequeBSmall = stopwatch.elapsedDuration();

        std::deque<int> stdQ;
        stopwatch.tick();
        {
            for (int i = 0; i < enqueuesize; ++i) {
                stdQ.push_back(i);
            }
        }
        stopwatch.tock();
        stdEnquequeBSmall = stopwatch.elapsedDuration();
    }

    /////////////////////////////////
    {
        Queue<BigE>      g3dQ;
        std::deque<BigE> stdQ;


        for (int i = 0; i < qsize; ++i) {
            BigE v;
            g3dQ.pushBack(v);
            stdQ.push_back(v);
        }

        // Run many times to filter out startup behavior
        for (int j = 0; j < 3; ++j) {
            BigE v = stdQ[0];

            stopwatch.tick();
            {
                for (int i = 0; i < iterations; ++i) {
                    g3dQ.popFront();
                    g3dQ.pushBack(v);
                }
            }
            stopwatch.tock();
            g3dStreamLarge = stopwatch.elapsedDuration();

            stopwatch.tick();
            {
                for (int i = 0; i < iterations; ++i) {
                    stdQ.pop_front();
                    stdQ.push_back(v);
                }
            }
            stopwatch.tock();
            stdStreamLarge = stopwatch.elapsedDuration();
        }
    }
    {
        Queue<BigE>      g3dQ;
        stopwatch.tick();
        {
            BigE v;
            for (int i = 0; i < enqueuesize; ++i) {
                g3dQ.pushFront(v);
            }
        }
        stopwatch.tock();
        g3dEnquequeFLarge = stopwatch.elapsedDuration();

        std::deque<BigE> stdQ;
        stopwatch.tick();
        {
            BigE v;
            for (int i = 0; i < enqueuesize; ++i) {
                stdQ.push_front(v);
            }
        }
        stopwatch.tock();
        stdEnquequeFLarge = stopwatch.elapsedDuration();
    }
    {
        Queue<BigE>      g3dQ;
        stopwatch.tick();
        BigE v;
        for (int i = 0; i < enqueuesize; ++i) {
            g3dQ.pushBack(v);
        }
        stopwatch.tock();
        g3dEnquequeBLarge = stopwatch.elapsedDuration();

        std::deque<BigE> stdQ;
        stopwatch.tick();
        {
            BigE v;
            for (int i = 0; i < enqueuesize; ++i) {
                stdQ.push_back(v);
            }
        }
        stopwatch.tock();
        stdEnquequeBLarge = stopwatch.elapsedDuration();
    }

    std::string header = "Pile-up push front (max queue size = " + std::to_string(enqueuesize) + ")";
    PRINT_HEADER(header.c_str());
    PRINT_MICRO("G3D::Queue<int>", "(us/elt)", g3dEnquequeFSmall / enqueuesize);
    PRINT_MICRO("std::deque<int>", "(us/elt)", stdEnquequeFSmall / enqueuesize);
    PRINT_MICRO("G3D::Queue<BigE>", "(us/elt)", g3dEnquequeFLarge / enqueuesize);
    PRINT_MICRO("std::deque<BigE>", "(us/elt)", stdEnquequeFLarge / enqueuesize);

    header = "Pile-up push back (max queue size = " + std::to_string(enqueuesize) + ")";
    PRINT_HEADER(header.c_str());
    PRINT_MICRO("G3D::Queue<int>", "(us/elt)", g3dEnquequeBSmall / enqueuesize);
    PRINT_MICRO("std::deque<int>", "(us/elt)", stdEnquequeBSmall / enqueuesize);
    PRINT_MICRO("G3D::Queue<BigE>", "(us/elt)", g3dEnquequeBLarge / enqueuesize);
    PRINT_MICRO("std::deque<BigE>", "(us/elt)", stdEnquequeBLarge / enqueuesize);

    header = "Streaming cycles (queue size = " + std::to_string(qsize) + ")";
    PRINT_HEADER(header.c_str());
    PRINT_MICRO("G3D::Queue<int>", "(us/iteration)", g3dStreamSmall / iterations);
    PRINT_MICRO("std::deque<int>", "(us/iteration)", stdStreamSmall / iterations);
    PRINT_MICRO("G3D::Queue<BigE>", "(us/iteration)", g3dStreamLarge / iterations);
    PRINT_MICRO("std::deque<BigE>", "(us/iteration)", stdStreamLarge / iterations);
}


static String makeMessage(Queue<int>& q) {
    String s = "Expected [ ";
    for (int i = 0; i < q.size(); ++i) {
        s = s + format("%d ", i);
    }
    s = s + "], got [";
    for (int i = 0; i < q.size(); ++i) {
        s = s + format("%d ", q[i]);
    }
    return s + "]";

}


static void _check(Queue<int>& q) {
    for (int i = 0; i < q.size(); ++i) {
        testAssertM(q[i] == (i + 1),
            makeMessage(q));
    }
}


static void testCopy() {
    Queue<int> q1, q2;
    for (int i = 0; i < 10; ++i) {
        q1.pushBack(i);
    }

    q2 = q1;

    for (int i = 0; i < 10; ++i) {
        testAssert(q2[i] == q1[i]);
    }
}


void testQueue() {
    printf("Queue ");

    testCopy();

    {
        Queue<int> q;
        q.pushFront(3);
        q.pushFront(2);
        q.pushFront(1);
        _check(q);
    }


    {
        Queue<int> q;
        q.pushBack(1);
        q.pushBack(2);
        q.pushBack(3);
        _check(q);
    }

    {
        Queue<int> q;
        q.pushFront(2);
        q.pushFront(1);
        q.pushBack(3);
        _check(q);
    }


    { 
        Queue<int> q;
        q.pushFront(2);
        q.pushBack(3);
        q.pushFront(1);
        _check(q);
    }

    {
        Queue<int> q;
        q.pushBack(2);
        q.pushFront(1);
        q.pushBack(3);
        _check(q);
    }

    {
        Queue<int> q;
        q.pushBack(-1);
        q.pushBack(1);
        q.pushBack(2);
        q.pushBack(3);
        q.pushBack(-1);

        q.popFront();
        q.popBack();
        _check(q);
    }

    {
        Queue<int> q;
        q.pushBack(1);
        q.pushBack(2);
        q.pushBack(3);
        q.pushBack(-1);

        q.popBack();
        _check(q);
    }
    {
        Queue<int> q;
        q.pushBack(-1);
        q.pushBack(1);
        q.pushBack(2);
        q.pushBack(3);
 
        q.popFront();
        _check(q);
    }

    // Sanity check queue copying.
    {
        Queue<int> q;
        q.pushBack(1);
        q.pushBack(2);
        q.pushBack(3);
 
        _check(q);

        Queue<int> r(q);
        _check(r);
    }
    
    printf("succeeded\n");
}

#undef check
