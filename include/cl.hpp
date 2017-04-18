//
// Created by janko on 10.4.2017.
//

#ifndef TESTAPP_CL_HPP
#define TESTAPP_CL_HPP

#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

namespace cl {

    /**
     * Point class represents 3D point with 'double' underlying type
     */
    class Point {
    public:
        /**
         * Create point with default values
         */
        Point() : _x(double()), _y(double()), _z(double()) {};

        /**
         * Create point with specified values
         * @param x
         * @param y
         * @param z
         */
        Point(double x, double y, double z) : _x(x), _y(y), _z(z) {};

        /**
         * Adds one point to another
         * @param point
         * @return new point
         */
        Point operator+(Point& point) const {
            Point p = *this;
            p._x += point.x();
            p._y += point.y();
            p._z += point.z();
            return p;
        }

        /**
         * Subtracts one point from another
         * @param point
         * @return new point
         */
        Point operator-(Point& point) const {
            Point p = *this;
            p._x -= point.x();
            p._y -= point.y();
            p._z -= point.z();
            return p;
        }

        /**
         * Multiplies one point by another
         * @param point
         * @return new point
         */
        Point operator*(Point& point) const {
            Point p = *this;
            p._x *= point.x();
            p._y *= point.y();
            p._z *= point.z();
            return p;
        }

        /**
         * Divides one point by another
         * @param point
         * @return new point
         */
        Point operator/(Point& point) const{
            Point p = *this;
            p._x /= point.x();
            p._y /= point.y();
            p._z /= point.z();
            return p;
        }

        /**
         * Friend stream operator
         * @param stream
         * @param point
         * @return
         */
        friend std::ostream& operator<<(std::ostream& stream, const Point& point);


        /**
         * Get x component
         * @return
         */
        double x() const {
            return _x;
        }

        /**
         * Get y component
         * @return
         */
        double y() const {
            return _y;
        }

        /**
         * Get z component
         * @return
         */
        double z() const {
            return _z;
        }

    private:
        double _x;
        double _y;
        double _z;
    };


    /**
     * Stream operator for Point type
     * @param stream
     * @param point
     * @return
     */
    std::ostream& operator<<(std::ostream& stream, const Point& point) {
        stream << "[ " << point._x << ", " << point._y << ", " << point._z << " ]\n";
        return stream;
    }

    /**
     * Class that represents point cloud
     */
    class PointCloud {
        using Points = std::vector<Point>;
    public:

        /**
         * Shared pointer to PointCloud
         */
        using Ptr = std::shared_ptr<PointCloud>;

        /**
         * add point to point cloud
         * @param point
         */
        void push_back(const Point& point)
        {
            _points.push_back(point);
        }

        /**
         * returns begin iterator of point cloud
         * @return
         */
        auto begin() const
        {
            return _points.begin();
        }

        /**
         * returns end iterator of point cloud
         * @return
         */
        auto end() const
        {
            return _points.end();
        }

        /**
         * returns size of point cloud
         * @return size of point cloud
         */
        auto size() const
        {
            return _points.size();
        }

        /**
         * returns point at specified position
         * @param index position of point in cloud
         * @return point
         */
        auto const & at(size_t index) const
        {
            return _points.at(index);
        }

        /**
         * Get pointer to data of underlaying container
         * @return
         */
        auto const data() const
        {
            return _points.data();
        }

        /**
         * Add one point cloud to another.
         * @param cloud
         * @return
         */
        PointCloud& operator+(const PointCloud& cloud){
            _points.insert(_points.begin(), cloud.begin(), cloud.end());
            return *this;
        }

        /**
         * Friend stream operator
         * @param stream
         * @param point
         * @return
         */
        friend std::ostream& operator<<(std::ostream& stream, const PointCloud& cloud);

    private:
        Points _points;
    };


    /**
     * Stream operator for Point type
     * @param stream
     * @param point
     * @return
     */
    std::ostream& operator<<(std::ostream& stream, const PointCloud& cloud) {
        stream << "CloudSize(" << cloud.size() << ") {\n";
        std::for_each(cloud.begin(), cloud.end(), [&](const auto& point){
            stream << "  " << point;
        });
        stream << "}\n";
    }

}

#endif //TESTAPP_CL_HPP
