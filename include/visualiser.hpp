#ifndef CL_VISUALISER_HPP
#define CL_VISUALISER_HPP

#include <memory>
#include "point_cloud.hpp"

namespace cl {
	class VisualiserImpl;

	class Visualiser {
	public:
		Visualiser();
		Visualiser(std::string name, int width = 800, int height = 600);
		~Visualiser();
		void addPointCloud(std::string cloudName, PointCloud::Ptr cloud);
		void spin();
	private:
		std::unique_ptr<VisualiserImpl> pimpl;
	};
}

#endif