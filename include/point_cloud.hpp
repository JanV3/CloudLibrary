#ifndef CL_POINT_CLOUD_HPP
#define CL_POINT_CLOUD_HPP

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

namespace cl {

    /**
    * Point class represents 3D point with 'double' underlying type
    */
    template <typename T>
    class PointXYZ {
    public:
        /**
        * Create point with default values
        */
        PointXYZ() : _x(T()), _y(T()), _z(T()){};

        /**
        * Create point with specified values
        * @param x
        * @param y
        * @param z
        */
        PointXYZ(T x, T y, T z) : _x(x), _y(y), _z(z){};

        /**
        * Adds one point to another
        * @param point
        * @return new point
        */
        PointXYZ<T> operator+(PointXYZ<T> &point) const
        {
            PointXYZ<T> p = *this;
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
        PointXYZ<T> operator-(PointXYZ<T> &point) const
        {
            PointXYZ<T> p = *this;
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
        PointXYZ<T> operator*(PointXYZ<T> &point) const
        {
            PointXYZ<T> p = *this;
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
        PointXYZ<T> operator/(PointXYZ<T> &point) const
        {
            PointXYZ<T> p = *this;
            p._x /= point.x();
            p._y /= point.y();
            p._z /= point.z();
            return p;
        }

        /**
        * Stream operator for Point type
        * @param stream
        * @param point
        * @return
        */
        friend std::ostream &operator<<(std::ostream &stream, const PointXYZ<T> &point)
        {
            stream << "[ " << point._x << ", " << point._y << ", " << point._z << " ]\n";
            return stream;
        }

        /**
        * Get x component
        * @return
        */
        T x() const
        {
            return _x;
        }

        /**
        * Get y component
        * @return
        */
        T y() const
        {
            return _y;
        }

        /**
        * Get z component
        * @return
        */
        T z() const
        {
            return _z;
        }

    private:
        T _x;
        T _y;
        T _z;
    };

    // Basic point alias
    using Point = PointXYZ<double>;

    /**
    * Class that represents point cloud
    */
    template <typename T>
    class PointCloudBase {
        using Points = std::vector<T>;

    public:
        /**
        * Shared pointer to PointCloud
        */
        using Ptr = std::shared_ptr<PointCloudBase<T>>;

        /**
        * add point to point cloud
        * @param point
        */
        void push_back(const T &point)
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
        auto const &at(size_t index) const
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
        PointCloudBase<T> &operator+(const PointCloudBase<T> &cloud)
        {
            _points.insert(_points.begin(), cloud.begin(), cloud.end());
            return *this;
        }

        /**
        * Stream operator for Point type
        * @param stream
        * @param point
        * @return
        */
        friend std::ostream &operator<<(std::ostream &stream, const PointCloudBase<T> &cloud)
        {
            stream << "CloudSize(" << cloud.size() << ") {\n";
            std::for_each(cloud.begin(), cloud.end(), [&](const auto &point) { stream << "  " << point; });
            stream << "}\n";
            return stream;
        }

    private:
        Points _points;
    };

    // Basic point cloud alias
    using PointCloud = PointCloudBase<Point>;
}

#endif // CL_POINT_CLOUD_HPP
