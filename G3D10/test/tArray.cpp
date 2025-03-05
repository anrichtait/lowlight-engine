/**
  \file test/tArray.cpp

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

void perfArray();
void testArray();


void compare(const SmallArray<int, 5>& small, const Array<int>& big) {
    testAssert(small.size() == big.size());
    for (int i = 0; i < small.size(); ++i) {
        testAssert(small[i] == big[i]);
    }
}

void testSmallArray() {
    printf("SmallArray...");

    SmallArray<int, 5> small;
    Array<int> big;

    for (int i = 0; i < 10; ++i) {
        small.push(i);
        big.push(i);
    }
    compare(small, big);

    for (int i = 0; i < 7; ++i) {
        int x = small.pop();
        int y = big.pop();
        testAssert(x == y);
    }
    compare(small, big);
    printf("passed\n");
}


class Big {
public:
    int x;
    /** Make this structure big */
    int dummy[100];

    Big() {
        /*
        x = 7;
        for (int i = 0; i < 100; ++i) {
            dummy[i] = i;
        }*/
    }

    ~Big() {
    }

    Big(const Big& a) : x(a.x) {
        /*
        for (int i = 0; i < 100; ++i) {
            dummy[i] = a.dummy[i];
        }*/
    }

    Big& operator=(const Big& a) {
        (void)a;
        /*
        x = a.x;
        for (int i = 0; i < 100; ++i) {
            dummy[i] = a.dummy[i];
        }
        */
        return *this;
    }
};

static void testIteration() {
    Array<int> array;
    array.append(100, 10, -10);

    {
        Array<int>::Iterator it = array.begin();
        int val = (*it);
        testAssert(val == 100);
        val = (*++it);
        testAssert(val == 10);
        val = (*++it);
        testAssert(val == -10);
    }

    {
        Array<int>::ConstIterator it = array.begin();
        int val = (*it);
        testAssert(val == 100);
        val = (*++it);
        testAssert(val == 10);
        val = (*++it);
        testAssert(val == -10);
    }

    // test stl helper typedefs
    {
        Array<int>::iterator it = array.begin();
        int val = (*it);
        testAssert(val == 100);
        val = (*++it);
        testAssert(val == 10);
        val = (*++it);
        testAssert(val == -10);
    }
}

static void testSort() {
    printf("Array::Sort\n");

    {
        Array<int> array;
        array.append(12, 7, 1);
        array.append(2, 3, 10);
    
        array.sort();

        testAssert(array[0] == 1);
        testAssert(array[1] == 2);
        testAssert(array[2] == 3);
        testAssert(array[3] == 7);
        testAssert(array[4] == 10);
        testAssert(array[5] == 12);
    }

    {
        Array<int> array;
        array.append(12, 7, 1);
        array.append(2, 3, 10);
    
        array.sortSubArray(0, 2);

        testAssert(array[0] == 1);
        testAssert(array[1] == 7);
        testAssert(array[2] == 12);
        testAssert(array[3] == 2);
        testAssert(array[4] == 3);
        testAssert(array[5] == 10);
    }
}


void testPartition() {

    // Create some array
    Array<int> array;
    array.append(4, -2, 7, 1);
    array.append(7, 13, 6, 8);
    array.append(-7, 7);

    Array<int> lt, gt, eq;

    // Partition
    int part = 7;
    array.partition(part, lt, eq, gt);

    // Ensure that the partition was correct
    for (int i = 0; i < lt.size(); ++i) {
        testAssert(lt[i] < part);
    }
    for (int i = 0; i < gt.size(); ++i) {
        testAssert(gt[i] > part);
    }
    for (int i = 0; i < eq.size(); ++i) {
        testAssert(eq[i] == part);
    }
    
    // Ensure that we didn't gain or lose elements
    Array<int> all;
    all.append(lt);
    all.append(gt);
    all.append(eq);

    array.sort();
    all.sort();
    testAssert(array.size() == all.size());
    for (int i = 0; i < array.size(); ++i) {
        testAssert(array[i] == all[i]);
    }
}


void testMedianPartition() {

    // Create an array
    Array<int> array;
    array.append(1, 2, 3, 4);
    array.append(5, 6, 7);
    array.randomize();

    Array<int> lt, gt, eq;

    array.medianPartition(lt, eq, gt);

    testAssert(lt.size() == 3);
    testAssert(eq.size() == 1);
    testAssert(gt.size() == 3);

    testAssert(eq.first() == 4);
    
    // Ensure that we didn't gain or lose elements
    Array<int> all;
    all.append(lt);
    all.append(gt);
    all.append(eq);

    array.sort();
    all.sort();
    testAssert(array.size() == all.size());
    for (int i = 0; i < array.size(); ++i) {
        testAssert(array[i] == all[i]);
    }

    // Test an even number of elements
    array.fastClear();
    array.append(1, 2, 3, 4);
    array.randomize();
    array.medianPartition(lt, eq, gt);
    testAssert(eq.first() == 2);
    testAssert(lt.size() == 1);
    testAssert(gt.size() == 2);

    // Test an even number of elements
    array.fastClear();
    array.append(1, 2, 3);
    array.append(4, 5, 6);
    array.randomize();
    array.medianPartition(lt, eq, gt);
    testAssert(eq.first() == 3);
    testAssert(lt.size() == 2);
    testAssert(gt.size() == 3);

    // Test with a repeated median element
    array.fastClear();
    array.append(1, 2, 4);
    array.append(4, 4, 7);
    array.randomize();
    array.medianPartition(lt, eq, gt);
    testAssert(eq.size() == 3);
    testAssert(eq.first() == 4);
    testAssert(lt.size() == 2);
    testAssert(gt.size() == 1);
}

void perfArrayAlloc() {
    // Note:
    //
    // std::vector calls the copy constructor for new elements and always calls the
    // constructor even when it doesn't exist (e.g., for int).  This makes its alloc
    // time much worse than other methods, but gives it a slight boost on the first
    // memory access because everything is in cache.  These tests work on huge arrays
    // to amortize that effect down.

    Stopwatch stopwatch;

    // Measure time for short array allocation
    chrono::nanoseconds vectorAllocBig, vectorAllocSmall;
    chrono::nanoseconds arrayAllocBig, arrayAllocSmall;

    const int M = 3000;

    // Run many times to filter out startup behavior
    for (int j = 0; j < 3; ++j) {
        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            std::vector<Big> v(4);
        }
        stopwatch.tock();
        vectorAllocBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            std::vector<int> v(4);
        }
        stopwatch.tock();
        vectorAllocSmall = stopwatch.elapsedDuration();

        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            Array<Big> v;
            v.resize(4);
        }
        stopwatch.tock();
        arrayAllocBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        for (int i = 0; i < M; ++i) {
            Array<int> v;
            v.resize(4);
        }
        stopwatch.tock();
        arrayAllocSmall = stopwatch.elapsedDuration();
    }

    PRINT_HEADER("Short Arrays");
    PRINT_TEXT("", "Big", "Small");
    PRINT_MICRO("G3D::Array", "(us)", arrayAllocBig / M, arrayAllocSmall / M);
    PRINT_MICRO("std::vector", "(us)", vectorAllocBig / M, vectorAllocSmall / M);
    PRINT_TEXT("Outcome", arrayAllocBig * 1.1 < vectorAllocBig ? "ok" : "FAIL", arrayAllocSmall * 1.1 < vectorAllocSmall ? "ok" : "FAIL");
}

void perfArrayResize() {
    Stopwatch stopwatch;

    ////////////////////////////////////////////////////
    // Measure time for array resize
    chrono::nanoseconds vectorResizeBig, vectorResizeSmall;
    chrono::nanoseconds arrayResizeBig, arrayResizeSmall;
    chrono::nanoseconds mallocResizeBig, mallocResizeSmall;

    const int M = 10000;

    //  Low and high bounds for resize tests
    const int L = 1;
    const int H = M + L;

    // Run many times to filter out startup behavior
    for (int j = 0; j < 3; ++j) {
        stopwatch.tick();
        {
            std::vector<Big> array;
            for (int i = L; i < H; ++i) {
                array.resize(i);
            }
        }
        stopwatch.tock();
        vectorResizeBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            std::vector<int> array;
            for (int i = L; i < H; ++i) {
                array.resize(i);
            }
        }
        stopwatch.tock();
        vectorResizeSmall = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            Array<int> array;
            for (int i = L; i < H; ++i) {
                array.resize(i, false);
            }
        }
        stopwatch.tock();
        arrayResizeSmall = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            Array<Big> array;
            for (int i = L; i < H; ++i) {
                array.resize(i, false);
            }
        }
        stopwatch.tock();
        arrayResizeBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            Big* array = NULL;
            for (int i = L; i < H; ++i) {
                array = (Big*)realloc(array, sizeof(Big) * i);
            }
            free(array);
        }
        stopwatch.tock();
        mallocResizeBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            int* array = NULL;
            for (int i = L; i < H; ++i) {
                array = (int*)realloc(array, sizeof(int) * i);
            }
            free(array);
        }
        stopwatch.tock();
        mallocResizeSmall = stopwatch.elapsedDuration();
    }

    PRINT_HEADER("Array resizes");
    PRINT_TEXT("", "Big", "Small");
    PRINT_MICRO("G3D::Array", "(us)", arrayResizeBig / M, arrayResizeSmall / M);
    PRINT_MICRO("std::vector", "(us)", vectorResizeBig / M, vectorResizeSmall / M);
    PRINT_MICRO("realloc", "(us)", mallocResizeBig / M, mallocResizeSmall / M);
    PRINT_TEXT("Outcome", arrayResizeBig < vectorResizeBig * 1.1 ? "ok" : "FAIL", arrayResizeSmall * 1.1 < vectorResizeSmall ? "ok" : "FAIL");
}

void perfArrayIntAccess() {
    Stopwatch stopwatch;

    // Measure times for various operations on large arrays of small elements
    chrono::nanoseconds newAllocInt, newFreeInt, newAccessInt;
    chrono::nanoseconds arrayAllocInt, arrayFreeInt, arrayAccessInt;
    chrono::nanoseconds vectorAllocInt, vectorFreeInt, vectorAccessInt;
    chrono::nanoseconds mallocAllocInt, mallocFreeInt, mallocAccessInt;
    chrono::nanoseconds sysmallocAllocInt, sysmallocFreeInt, sysmallocAccessInt;

    // The code that generates memory accesses
#define LOOPS\
            for (int k = 0; k < 3; ++k) {\
                int i;\
		        for (i = 0; i < size; ++i) {\
                    array[i] = i;\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i];\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i];\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i];\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i];\
                }\
            }

    // 10 million
    int size = 10000000;

    // Run many times to filter out startup behavior
    for (int j = 0; j < 3; ++j) {
        stopwatch.tick();
        {
            int* array = (int*)malloc(sizeof(int) * size);
            stopwatch.tock();
            mallocAllocInt = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            mallocAccessInt = stopwatch.elapsedDuration();
            stopwatch.tick();
            free(array);
        }
        stopwatch.tock();
        mallocFreeInt = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            int* array = (int*)System::alignedMalloc(sizeof(int) * size, 4096);
            stopwatch.tock();
            sysmallocAllocInt = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            sysmallocAccessInt = stopwatch.elapsedDuration();
            stopwatch.tick();
            System::alignedFree(array);
        }
        stopwatch.tock();
        sysmallocFreeInt = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            Array<int> array;
            array.resize(size);
            stopwatch.tock();
            arrayAllocInt = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            arrayAccessInt = stopwatch.elapsedDuration();
            stopwatch.tick();
        }
        stopwatch.tock();
        arrayFreeInt = stopwatch.elapsedDuration();

        {
            stopwatch.tick();
            int* array = new int[size];
            stopwatch.tock();
            newAllocInt = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            newAllocInt = stopwatch.elapsedDuration();

            stopwatch.tick();
            delete[] array;
        }
        stopwatch.tock();
        newFreeInt = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            std::vector<int> array(size);
            stopwatch.tock();
            vectorAllocInt = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            vectorAccessInt = stopwatch.elapsedDuration();
            stopwatch.tick();
        }
        stopwatch.tock();
        vectorFreeInt = stopwatch.elapsedDuration();
    }

#undef LOOPS

    // Number of memory ops per element that LOOPS performed
    const int N = 9 * 3;

    PRINT_HEADER("int array access");
    PRINT_TEXT("", "Alloc", "Access", "Free");
    PRINT_MICRO("G3D::Array", "(us/elt)", (arrayAllocInt / size), (arrayAccessInt / (N * size)), (arrayFreeInt / size));
    PRINT_MICRO("std::vector", "(us/elt)", (vectorAllocInt / size), (vectorAccessInt / (N * size)), (vectorFreeInt / size));
    PRINT_MICRO("new/delete", "(us/elt)", (newAllocInt / size), (newAccessInt / (N * size)), (newFreeInt / size));
    PRINT_MICRO("malloc/free", "(us/elt)", (mallocAllocInt / size), (mallocAccessInt / (N * size)), (mallocFreeInt / size));
    PRINT_MICRO("System::alignedMalloc", "(us/elt)", (sysmallocAllocInt / size), (sysmallocAccessInt / (N * size)), (sysmallocFreeInt / size));
}

void perfArrayBigAccess() {
    Stopwatch stopwatch;

    ///////////////////////////////////////////////////////
    // The code that generates memory accesses
#   define LOOPS\
            for (int k = 0; k < 3; ++k) {\
                int i;\
		        for (i = 0; i < size; ++i) {\
                    array[i].x = i;\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i].x;\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i].x;\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i].x;\
                }\
                for (i = 0; i < size; ++i) {\
                    ++array[i].x;\
                }\
            }
    // 1 million
    int size = 1000000;

    // Measure times for various operations on large arrays of small elements
    chrono::nanoseconds newAllocBig, newFreeBig, newAccessBig;
    chrono::nanoseconds arrayAllocBig, arrayFreeBig, arrayAccessBig;
    chrono::nanoseconds vectorAllocBig, vectorFreeBig, vectorAccessBig;
    chrono::nanoseconds mallocAllocBig, mallocFreeBig, mallocAccessBig;
    chrono::nanoseconds sysmallocAllocBig, sysmallocFreeBig, sysmallocAccessBig;

    // Run many times to filter out startup behavior
    for (int j = 0; j < 3; ++j) {
        stopwatch.tick();
        {
            Big* array = (Big*)malloc(sizeof(Big) * size);
            stopwatch.tock();
            mallocAllocBig = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            mallocAccessBig = stopwatch.elapsedDuration();
            stopwatch.tick();
            free(array);
        }
        stopwatch.tock();
        mallocFreeBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            Big* array = (Big*)System::alignedMalloc(sizeof(Big) * size, 4096);
            stopwatch.tock();
            sysmallocAllocBig = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            sysmallocAccessBig = stopwatch.elapsedDuration();
            stopwatch.tick();
            System::alignedFree(array);
        }
        stopwatch.tock();
        sysmallocFreeBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            Array<Big> array;
            array.resize(size);
            stopwatch.tock();
            arrayAllocBig = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            arrayAccessBig = stopwatch.elapsedDuration();
            stopwatch.tick();
        }
        stopwatch.tock();
        arrayFreeBig = stopwatch.elapsedDuration();

        {
            stopwatch.tick();
            Big* array = new Big[size];
            stopwatch.tock();
            newAllocBig = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            newAccessBig = stopwatch.elapsedDuration();

            stopwatch.tick();
            delete[] array;
        }
        stopwatch.tock();
        newFreeBig = stopwatch.elapsedDuration();

        stopwatch.tick();
        {
            std::vector<Big> array;
            array.resize(size);
            stopwatch.tock();
            vectorAllocBig = stopwatch.elapsedDuration();
            stopwatch.tick();
            LOOPS;
            stopwatch.tock();
            vectorAccessBig = stopwatch.elapsedDuration();
            stopwatch.tick();
        }
        stopwatch.tock();
        vectorFreeBig = stopwatch.elapsedDuration();
    }

#   undef LOOPS

    // Number of memory ops per element that LOOPS performed (3 loops * (5 writes + 4 reads))
    const int N = 9 * 3;

    PRINT_HEADER("Big class array access");
    PRINT_TEXT("", "Alloc", "Access", "Free");
    PRINT_MICRO("G3D::Array", "(us/elt)", (arrayAllocBig / size), (arrayAccessBig / (N * size)), (arrayFreeBig / size));
    PRINT_MICRO("std::vector", "(us/elt)", (vectorAllocBig / size), (vectorAccessBig / (N * size)), (vectorFreeBig / size));
    PRINT_MICRO("new/delete", "(us/elt)", (newAllocBig / size), (newAccessBig / (N * size)), (newFreeBig / size));
    PRINT_MICRO("malloc/free", "(us/elt)", (mallocAllocBig / size), (mallocAccessBig / (N * size)), (mallocFreeBig / size));
    PRINT_MICRO("System::alignedMalloc", "(us/elt)", (sysmallocAllocBig / size), (sysmallocAccessBig / (N * size)), (sysmallocFreeBig / size));
}

void perfArray() {
    PRINT_SECTION("Performance: Array", "Checks performance of G3D::Array against standard library");

    perfArrayAlloc();
    perfArrayResize();
    perfArrayIntAccess();
    perfArrayBigAccess();
}


void testParams() {
    // Make sure this compiles and links
    Array<int, 0> packed;
    packed.append(1);
    packed.append(2);
    packed.clear();
}

void testArray() {
    printf("G3D::Array  ");
    testIteration();
    testPartition();
    testMedianPartition();
    testSort();
    testParams();
    printf("passed\n");
}
