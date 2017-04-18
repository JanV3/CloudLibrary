#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "cl.hpp"

TEST_CASE("Add two _points"){
    cl::Point p1{1.0, 2.0, 3.0};
    cl::Point p2{3.0, 2.0, 1.0};
    auto p3 = p2 + p1;

    REQUIRE(p3.x() == 4.0);
    REQUIRE(p3.y() == 4.0);
    REQUIRE(p3.z() == 4.0);
}

TEST_CASE("Subtract two _points") {
    cl::Point p1{1.0, 2.0, 3.0};
    cl::Point p2{3.0, 2.0, 1.0};
    auto p3 = p2 - p1;

    REQUIRE(p3.x() == 2.0);
    REQUIRE(p3.y() == 0.0);
    REQUIRE(p3.z() == -2.0);
}

TEST_CASE("Divide two _points") {
    cl::Point p1{1.0, 2.0, 2.0};
    cl::Point p2{4.0, 2.0, 1.0};
    auto p3 = p2 / p1;

    REQUIRE(p3.x() == 4.0);
    REQUIRE(p3.y() == 1.0);
    REQUIRE(p3.z() == 0.5);
}

TEST_CASE("Multiplies two _points") {
    cl::Point p1{1.0, 2.0, 2.0};
    cl::Point p2{4.0, 2.0, 1.0};
    auto p3 = p2 * p1;

    REQUIRE(p3.x() == 4.0);
    REQUIRE(p3.y() == 4.0);
    REQUIRE(p3.z() == 2.0);
}