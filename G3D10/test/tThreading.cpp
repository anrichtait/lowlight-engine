/**
  \file test/tThreading.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D/G3D.h"
#include "testassert.h"
#include <mutex>
#include <thread>

Spinlock spinLock;
int spinlockValue = 0;

std::atomic_bool startTest = ATOMIC_VAR_INIT(false);

void testSpinlock() {
    std::thread thread1 = std::thread([]() {
        while (!startTest) {};
        for (int i = 0; i < 3000; ++i) {
            spinLock.lock();
            ++spinlockValue;
            spinLock.unlock();
        }
    });
    std::thread thread2 = std::thread([]() {
        while (!startTest) {};
        for (int i = 0; i < 3000; ++i) {
            spinLock.lock();
            ++spinlockValue;
            spinLock.unlock();
        }
    });
    std::thread thread3 = std::thread([]() {
        for (int i = 0; i < 3000; ++i) {
            while (!startTest) {};
            spinLock.lock();
            ++spinlockValue;
            spinLock.unlock();
        }
    });

    startTest = true;
    thread1.join();
    thread2.join();
    thread3.join();
    spinLock.lock();
    testAssert(spinlockValue == 9000);
    spinLock.unlock();
}

void testThread() {

    printf("G3D::Spinlock ");

    {
        testSpinlock();
    }

    printf("passed\n");
}

