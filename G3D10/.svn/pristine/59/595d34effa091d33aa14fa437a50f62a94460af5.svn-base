#include <cmath>
#include <limits>

#include "G3D/G3D.h"
#include "main.h"
#include "gtest/gtest.h"


TEST(G3DMath, NumericLimits) {
    /* When compiling  with the -ffast-math optimization, the following
       two tests will fail since fpclassify returns G3D::inf() as
       FP_NORMAL */
    EXPECT_EQ(std::fpclassify(G3D::inf()), FP_INFINITE);
    EXPECT_EQ(std::fpclassify(G3D::finf()), FP_INFINITE);
    EXPECT_FALSE(G3D::inf() < G3D::inf());
    EXPECT_FALSE(G3D::finf() < G3D::finf());
}
