#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <Binpack2D/binpack2d.hpp>


// Simple test, does not use gmock
TEST(Dummy, foobar)
{

    binpack2d::Rectangle rect;
    binpack2d::Packager packager;

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
