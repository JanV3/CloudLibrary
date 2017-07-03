#ifndef CL_ALGORITHMS_HPP
#define CL_ALGORITHMS_HPP

#include <numeric>

#include "point_cloud.hpp"

namespace cl {
    template <typename T, typename P = typename T::type>
    P centroid(T cloud)
    {
        return std::accumulate(cloud.begin(), cloud.end(), P()) / cloud.size();
    }


	template <typename T>
	bool compareRealNumber(T a, T b)
	{
		T diff = a - b;
		return (diff < 0.01) && (-diff < 0.01);
	}

	template<typename T>
	T median(std::vector<T>& values)
	{
		T median = 0.0;
		if (values.empty())
			return median;

		std::sort(values.begin(), values.end());
		size_t sizeHalf = values.size() / 2;
		if (values.size() % 2 == 0) {
			median = (values[sizeHalf - 1] + values[sizeHalf]) / 2.0f;
		}
		else {
			median = values[sizeHalf];
		}
		return median;
	}

	template<typename T>
	T mean(std::vector<T>& values)
	{
		T mean = 0.0;
		if (values.empty())
			return mean;

		for (auto d = values.begin(); d != values.end(); ++d)
			mean += *d;
		mean /= (T)values.size();

		return mean;
	}

	// filter point cloud by neighboors distance
    // FIXME: points must be all indices of cloud, otherwise the algorithm is not correct
	void noiseFilter(
		PointCloud::Ptr& cloud,
		PointIndices& points,
		PointIndices& filteredPoints,
		unsigned int windowSize,
		float rangeThreshold)
	{
        if (!cloud->isOrganized())
            throw std::runtime_error("NoiseFilter cannot be applied to non-organized point cloud.");

        auto width = static_cast<int>(cloud->getWidth());
        auto height = static_cast<int>(cloud->getHeight());

		auto halfWindow = static_cast<int>(windowSize / 2);

		for (const auto& p : points) {
			int row = p / width;
			int column = p % width;

			int fromColumn = column - halfWindow;
			int toColumn = column + halfWindow;
			int fromRow = row - halfWindow;
			int toRow = row + halfWindow;

			if (fromColumn < 0)
				fromColumn = 0;
			if (toColumn > width)
				toColumn = width;
			if (fromRow < 0)
				fromRow = 0;
			if (toRow > height)
				toRow = height;

			// calculate median of ranges from points in widnow and add threshold value
			std::vector<float> ranges;
			for (size_t r = fromRow; r < toRow; r++) {
				for (size_t c = fromColumn; c < toColumn; c++) {
					size_t pIndex = (r * width) + c;
					ranges.push_back(cloud->at(pIndex).z);
				}
			}
			float medianRange = median(ranges);
			float positiveThr = medianRange + rangeThreshold;
			float negativeThr = medianRange - rangeThreshold;
			if (cloud->at(p).z < positiveThr && cloud->at(p).z > negativeThr)
				filteredPoints.push_back(p);
		}
	}

}

#endif // !CL_ALGORITHMS_HPP
