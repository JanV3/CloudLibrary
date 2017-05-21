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
}

#endif // !CL_ALGORITHMS_HPP
