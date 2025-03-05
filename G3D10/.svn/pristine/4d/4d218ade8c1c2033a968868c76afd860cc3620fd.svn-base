/**
  \file G3D-base.lib/source/Thread.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/Thread.h"
#include "G3D-base/System.h"
#include "G3D-base/debugAssert.h"

namespace G3D {

// Below this size, we use parallel_for on individual elements
static const int TASKS_PER_BATCH = 32;

void runConcurrently
   (const Point3int32& start, 
    const Point3int32& stopBefore, 
    const std::function<void (Point3int32)>& callback,
    bool singleThread) {

    const Point3int32 extent = stopBefore - start;
    const int numTasks = extent.x * extent.y * extent.z;
    const int numRows = extent.y * extent.z;

    if (singleThread) {
        for (Point3int32 coord(start); coord.z < stopBefore.z; ++coord.z) {
            for (coord.y = start.y; coord.y < stopBefore.y; ++coord.y) {
                for (coord.x = start.x; coord.x < stopBefore.x; ++coord.x) {
                    callback(coord);
                }
            }
        }
    } else if (extent.x > TASKS_PER_BATCH) {
        // Group tasks into batches by row (favors Y; blocks would be better)
        tbb::parallel_for(0, numRows, [&](size_t r) {
            for (Point3int32 coord(start.x, (int(r) % extent.y) + start.y, (int(r) / extent.y) + start.z); coord.x < stopBefore.x; ++coord.x) {
                callback(coord);
            }
        });
    } else if (extent.x * extent.y > TASKS_PER_BATCH) {
        // Group tasks into batches by groups of rows (favors Z; blocks would be better)
        tbb::parallel_for(tbb::blocked_range<size_t>(0, numRows, TASKS_PER_BATCH), [&](const tbb::blocked_range<size_t>& block) {
            for (size_t r = block.begin(); r < block.end(); ++r) {
                for (Point3int32 coord(start.x, (int(r) % extent.y) + start.y, (int(r) / extent.y) + start.z); coord.x < stopBefore.x; ++coord.x) {
                    callback(coord);
                }
            }
        });
    } else {
        // Process individual tasks as their own batches
        const int tasksPerPlane = extent.x * extent.y;
        tbb::parallel_for(0, numTasks, 1, [&](size_t i) {
            const int t = (int(i) % tasksPerPlane); 
            callback(Point3int32((t % extent.x) + start.x, (t / extent.x) + start.y, (int(i) / tasksPerPlane) + start.z));
        });
    }
}


void runConcurrently
   (const Point2int32& start,
    const Point2int32& stopBefore, 
    const std::function<void (Point2int32)>& callback,
    bool singleThread) {

    const Point2int32 extent = stopBefore - start;
    const int numTasks = extent.x * extent.y;
    
    if (singleThread) {
        for (Point2int32 coord(start); coord.y < stopBefore.y; ++coord.y) {
            for (coord.x = start.x; coord.x < stopBefore.x; ++coord.x) {
                callback(coord);
            }
        }
    } else if (extent.y > TASKS_PER_BATCH) {
        // Group tasks into batches by row (favors Y; blocks would be better)
        tbb::parallel_for(start.y, stopBefore.y, 1, [&](size_t y) {
            for (Point2int32 coord(start.x, int(y)); coord.x < stopBefore.x; ++coord.x) {
                callback(coord);
            }
        });
    } else if (extent.x > TASKS_PER_BATCH) {
        // Group tasks into batches by column
        tbb::parallel_for(start.x, stopBefore.x, 1, [&](size_t x) {
            for (Point2int32 coord(int(x), start.y); coord.y < stopBefore.y; ++coord.y) {
                callback(coord);
            }
        });
    } else {
        // Process individual tasks as their own batches
        tbb::parallel_for(0, numTasks, 1, [&](size_t i) {
            callback(Point2int32((int(i) % extent.x) + start.x, (int(i) / extent.x) + start.y));
        });
    }
}


void runConcurrently
   (const int& start, 
    const int& stopBefore, 
    const std::function<void (int)>& callback,
    bool singleThread) {

    if (singleThread) {
        for (int i = start; i < stopBefore; ++i) {
            callback(i);
        }
    } else if (stopBefore - start > TASKS_PER_BATCH) {
        // Group tasks into batches
        tbb::parallel_for(tbb::blocked_range<size_t>(start, stopBefore, TASKS_PER_BATCH), [&](const tbb::blocked_range<size_t>& block) {
            for (size_t i = block.begin(); i < block.end(); ++i) {
                callback(int(i));
            }
        });
    } else {
        // Process individual tasks as their own batches
        tbb::parallel_for(start, stopBefore, 1, [&](size_t i) {
            callback(int(i));
        });
    }
}


void runConcurrently
   (const size_t& start, 
    const size_t& stopBefore, 
    const std::function<void (size_t)>& callback,
    bool singleThread) {

    if (singleThread) {
        for (size_t i = start; i < stopBefore; ++i) {
            callback(i);
        }
    } else if (stopBefore - start > TASKS_PER_BATCH) {
        // Group tasks into batches
        tbb::parallel_for(tbb::blocked_range<size_t>(start, stopBefore, TASKS_PER_BATCH), [&](const tbb::blocked_range<size_t>& block) {
            for (size_t i = block.begin(); i < block.end(); ++i) {
                callback(i);
            }
        });
    } else {
        // Process individual tasks as their own batches
        tbb::parallel_for<size_t>(start, stopBefore, 1, [&](size_t i) {
            callback(i);
        });
    }
}

} // namespace G3D
