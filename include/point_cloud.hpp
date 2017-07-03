#ifndef CL_POINT_CLOUD_HPP
#define CL_POINT_CLOUD_HPP

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cmath>

namespace cl {

    /**
    * Compare two real values
    */
    template <class T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type compare(T x, T y, int ulp = 10)
    {
        return std::fabs(x - y) < std::numeric_limits<T>::epsilon() * std::fabs(x + y) * ulp
               || std::fabs(x - y) < std::numeric_limits<T>::min();
    }

    /**
    * Templated point class that represents 3D point
    */
    template <typename T>
    struct PointXYZ {
        /**
        * Create point with default values
        */
        PointXYZ() : x(T(0.0)), y(T(0.0)), z(T(0.0)){};

        /**
        * Create point with specified values
        * @param x
        * @param y
        * @param z
        */
        PointXYZ(T xx, T yy, T zz) : x(xx), y(yy), z(zz){};

        // X axis value
        T x;

        // Y axis value
        T y;

        // Z axis value
        T z;
    };

    /**
    * Stream operator for Point type
    * @param stream
    * @param point
    * @return
    */
    template <typename T>
    std::ostream &operator<<(std::ostream &stream, const PointXYZ<T> &point)
    {
        stream << "[ " << point.x << ", " << point.y << ", " << point.z << " ]\n";
        return stream;
    }

    /**
    * Adds one point to another
    * @param point
    * @return new point
    */
    template <typename T>
    auto operator+(const PointXYZ<T> &p1, const PointXYZ<T> &p2)
    {
        PointXYZ<T> p3;
        p3.x = p1.x + p2.x;
        p3.y = p1.y + p2.y;
        p3.z = p1.z + p2.z;
        return p3;
    }

    /**
    * Subtracts one point from another
    * @param point
    * @return new point
    */
    template <typename T>
    auto operator-(const PointXYZ<T> &p1, const PointXYZ<T> &p2)
    {
        PointXYZ<T> p3;
        p3.x = p1.x - p2.x;
        p3.y = p1.y - p2.y;
        p3.z = p1.z - p2.z;
        return p3;
    }

    /**
    * Multiplies one point by another
    * @param point
    * @return new point
    */
    template <typename T>
    auto operator*(const PointXYZ<T> &p1, const PointXYZ<T> &p2)
    {
        PointXYZ<T> p3;
        p3.x = p1.x * p2.x;
        p3.y = p1.y * p2.y;
        p3.z = p1.z * p2.z;
        return p3;
    }

    /**
    * Divides one point by another
    * @param point
    * @return new point
    */
    template <typename T>
    auto operator/(const PointXYZ<T> &p1, const PointXYZ<T> &p2)
    {
        PointXYZ<T> p3;
        p3.x = p1.x / p2.x;
        p3.y = p1.y / p2.y;
        p3.z = p1.z / p2.z;
        return p3;
    }

    /**
    * Divides one point by scalar value
    * @param scalar value
    * @return new point
    */
    template <typename T, typename D>
    auto operator/(const PointXYZ<T> &p1, const D &value)
    {
        PointXYZ<T> p3;
        p3.x = p1.x / static_cast<T>(value);
        p3.y = p1.y / static_cast<T>(value);
        p3.z = p1.z / static_cast<T>(value);
        return p3;
    }

    /**
    * Compare two points
    * @param point
    * @return true when points are same, otherwise false
    */
    template <typename T>
    auto operator==(const PointXYZ<T> &p1, const PointXYZ<T> &p2)
    {
        return compare(p1.x, p2.x) && compare(p1.y, p2.y) && compare(p1.z, p2.z);
    }

    /**
    * Class that represents point cloud
    */
    template <typename T>
    class PointCloudBase {
        using Points = std::vector<T>;

    public:
        /**
        * Type of underlying point type
        */
        using type = T;

        /**
        * Shared pointer to PointCloud
        */
        using Ptr = std::shared_ptr<PointCloudBase<T>>;

        /**
        * Default constructor
        */
        PointCloudBase<T>()
            : width_(0)
            , height_(0)
        {}

        /**
        * Constructor with name and/or width and height of point cloud
        */
		PointCloudBase<T>(std::string name, size_t width = 0, size_t height = 0)
			: name_(name)
			, width_(width)
			, height_(height)
		{
		}

        /**
        * If cloud have non-zero width or height,
        * it is considered as organized.
        */
		bool isOrganized() const
		{
			if (width_ != 0 || height_ != 0)
				return true;
			return false;
		}

        /**
        * Set point cloud width.
        * If width is non-zero, point cloud is organized.
        */
        void setWidth(size_t width)
        {
            width_ = width;
        }

        /**
        * Set point cloud height.
        * If height is non-zero, point cloud is organized.
        */
        void setHeight(size_t height)
        {
            height_ = height;
        }

        /**
        * Returns width of point cloud.
        * If width is non-zero, point cloud is considered as organized.
        */
		size_t getWidth() const
		{
			return width_;
		}

        /**
        * Returns height of point cloud.
        * If height is non-zero, point cloud is considered as organized.
        */
		size_t getHeight() const
		{
			return height_;
		}

        /**
        * add point to point cloud
        * @param point
        */
        void push_back(const T &point)
        {
            points_.push_back(point);
        }

        /**
        * returns begin iterator of point cloud
        * @return
        */
        auto begin() const
        {
            return points_.begin();
        }

        /**
        * returns end iterator of point cloud
        * @return
        */
        auto end() const
        {
            return points_.end();
        }

        /**
        * returns size of point cloud
        * @return size of point cloud
        */
        auto size() const
        {
            return points_.size();
        }

		/**
		* Resize point cloud to specific size
		*/
		void resize(size_t size)
		{
			points_.resize(size);
		}

        /**
		* Check if point cloud empty
		* @return true if empty
		*/	
        auto empty() const
        {
            return points_.empty();
        }

        /**
        * returns point at specified position
        * @param index position of point in cloud
        * @return point
        */
        auto &at(size_t index)
        {
            return points_.at(index);
        }

        /**
        * Get pointer to data of underlaying container
        * @return
        */
        auto data()
        {
            return points_.data();
        }

        /** Get name of point cloud. If name is not set, returns empty string */
		auto getName() const
		{
			return name_;
		}

        /** Set point cloud name */
		void setName(std::string name)
		{
			name_ = name;
		}

        /**
        * Add one point cloud to another.
        * @param cloud
        * @return
        */
        PointCloudBase<T> &operator+(const PointCloudBase<T> &cloud)
        {
            points_.insert(points_.begin(), cloud.begin(), cloud.end());
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
        Points points_;
		std::string name_;
		size_t width_;
		size_t height_;
    };

    // Basic point alias
    using Point = PointXYZ<float>;

    // Basic point cloud alias
    using PointCloud = PointCloudBase<Point>;

    // Point indices
	using PointIndices = std::vector<int>;
}

#endif // CL_POINT_CLOUD_HPP