/*
X11 has macros named "None" and "Bool" which conflict with the None struct 
located in googletest/include/gtest/internal/gtest-type-util.h on line 112 
and the Bool function in googletest/include/gtest/gtest-param-test.h on line 
359. This wrapper header solves the conflict by undefining the macros before 
including gtest/gtest.h in our source files.
*/

#ifndef G3D_UNITTEST_H
#define G3D_UNITTEST_H

#ifdef None
#undef None
#endif

#ifdef Bool
#undef Bool
#endif

//#include "gtest/gtest.h"

#endif // G3D_UNITTEST_H
