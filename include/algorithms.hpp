#ifndef CL_ALGORITHMS_HPP
#define CL_ALGORITHMS_HPP

#include <numeric>

#include "point_cloud.hpp"

namespace cl {

	/* TODO: make it tempalted
	template<typename T>
	T centroid(PointCloudBase<T>::Ptr cloud) {
		return std::accumulate(cloud->begin(), cloud->end(), T()) / cloud->size();
	}*/

	Point centroid(PointCloud::Ptr cloud){
		return std::accumulate(cloud->begin(), cloud->end(), Point()) / cloud->size();
	}
}

#endif // !CL_ALGORITHMS_HPP

