#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

#include "point_cloud.hpp"

namespace cl {
	// POINT IMPLEMENTATION
	Point::Point() : _x(double()), _y(double()), _z(double()) {};

	Point::Point(double x, double y, double z) : _x(x), _y(y), _z(z) {};

	Point Point::operator+(Point& point) const {
		Point p = *this;
		p._x += point.x();
		p._y += point.y();
		p._z += point.z();
		return p;
	}

	Point Point::operator-(Point& point) const {
		Point p = *this;
		p._x -= point.x();
		p._y -= point.y();
		p._z -= point.z();
		return p;
	}

	Point Point::operator*(Point& point) const {
		Point p = *this;
		p._x *= point.x();
		p._y *= point.y();
		p._z *= point.z();
		return p;
	}

	Point Point::operator/(Point& point) const {
		Point p = *this;
		p._x /= point.x();
		p._y /= point.y();
		p._z /= point.z();
		return p;
	}

	double Point::x() const {
		return _x;
	}

	double Point::y() const {
		return _y;
	}

	double Point::z() const {
		return _z;
	}

	std::ostream& operator<<(std::ostream& stream, const Point& point) {
		stream << "[ " << point._x << ", " << point._y << ", " << point._z << " ]\n";
		return stream;
	}

	// POINT CLOUD IMPLEMENTATION
	void PointCloud::push_back(const Point& point)
	{
		_points.push_back(point);
	}

	auto PointCloud::begin() const
	{
		return _points.begin();
	}


	auto PointCloud::end() const
	{
		return _points.end();
	}


	auto PointCloud::size() const
	{
		return _points.size();
	}


	auto const & PointCloud::at(size_t index) const
	{
		return _points.at(index);
	}


	auto const PointCloud::data() const
	{
		return _points.data();
	}

	PointCloud& PointCloud::operator+(const PointCloud& cloud) {
		_points.insert(_points.begin(), cloud.begin(), cloud.end());
		return *this;
	}

	std::ostream& operator<<(std::ostream& stream, const PointCloud& cloud) {
		stream << "CloudSize(" << cloud.size() << ") {\n";
		std::for_each(cloud.begin(), cloud.end(), [&](const auto& point) {
			stream << "  " << point;
		});
		stream << "}\n";
		return stream;
	}
}
