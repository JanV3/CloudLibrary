#define CATCH_CONFIG_MAIN
#include "algorithms.hpp"
#include "catch.hpp"
#include "point_cloud.hpp"
#include "io.hpp"

TEST_CASE("Add two points")
{
    cl::Point p1{1.0, 2.0, 3.0};
    cl::Point p2{3.0, 2.0, 1.0};
    auto p3 = p2 + p1;

    REQUIRE(p3.x == 4.0);
    REQUIRE(p3.y == 4.0);
    REQUIRE(p3.z == 4.0);
}

TEST_CASE("Subtract two points")
{
    cl::Point p1{1.0, 2.0, 3.0};
    cl::Point p2{3.0, 2.0, 1.0};
    auto p3 = p2 - p1;

    REQUIRE(p3.x == 2.0);
    REQUIRE(p3.y == 0.0);
    REQUIRE(p3.z == -2.0);
}

TEST_CASE("Divide two points")
{
    cl::Point p1{1.0, 2.0, 2.0};
    cl::Point p2{4.0, 2.0, 1.0};
    auto p3 = p2 / p1;

    REQUIRE(p3.x == 4.0);
    REQUIRE(p3.y == 1.0);
    REQUIRE(p3.z == 0.5);
}

TEST_CASE("Multiplies two points")
{
    cl::Point p1{1.0, 2.0, 2.0};
    cl::Point p2{4.0, 2.0, 1.0};
    auto p3 = p2 * p1;

    REQUIRE(p3.x == 4.0);
    REQUIRE(p3.y == 4.0);
    REQUIRE(p3.z == 2.0);
}

TEST_CASE("Compute centroid")
{
    auto cloud = std::make_shared<cl::PointCloud>();
    cloud->push_back(cl::Point(1.0, 2.0, 3.0));
    cloud->push_back(cl::Point(3.0, 4.0, 5.0));
    auto centroid = cl::centroid(*cloud);

    REQUIRE(centroid == cl::Point(2.0, 3.0, 4.0));
}

TEST_CASE("Write and read 4 clouds")
{
	std::vector<cl::PointCloud::Ptr> clouds{
		std::make_shared<cl::PointCloud>(),
		std::make_shared<cl::PointCloud>("Camera 1"),
		std::make_shared<cl::PointCloud>("Camera 2"),
		std::make_shared<cl::PointCloud>("Camera 3")
	};
	clouds[1]->push_back({ 0.0f, 0.1f, 0.2f });

	clouds[2]->push_back({ 0.0f, 0.1f, 0.2f });
	clouds[2]->push_back({ 0.0f, 0.1f, 0.2f });

	clouds[3]->push_back({ 0.0f, 0.1f, 0.2f });
	clouds[3]->push_back({ 0.0f, 0.1f, 0.2f });
	clouds[3]->push_back({ 0.0f, 0.1f, 0.2f });

	cl::io::saveToBin("test.bin", clouds);
	cl::io::loadFromBin("test.bin", clouds);

	REQUIRE(clouds.size() == 8);
	CHECK(clouds[0]->getName() == clouds[4]->getName());
	CHECK(clouds[1]->getName() == clouds[5]->getName());
	CHECK(clouds[2]->getName() == clouds[6]->getName());
	CHECK(clouds[3]->getName() == clouds[7]->getName());

	REQUIRE(clouds[0]->size() == clouds[4]->size());
	REQUIRE(clouds[1]->size() == clouds[5]->size());
	REQUIRE(clouds[2]->size() == clouds[6]->size());
	REQUIRE(clouds[3]->size() == clouds[7]->size());

	REQUIRE(clouds[1]->at(0) == clouds[5]->at(0));

	REQUIRE(clouds[2]->at(0) == clouds[6]->at(0));
	REQUIRE(clouds[2]->at(0) == clouds[6]->at(0));

	REQUIRE(clouds[3]->at(0) == clouds[7]->at(0));
	REQUIRE(clouds[3]->at(0) == clouds[7]->at(0));
	REQUIRE(clouds[3]->at(0) == clouds[7]->at(0));
}