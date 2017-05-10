#include "visualiser.hpp"
#include "visualiser_impl.hpp"

namespace cl {
	Visualiser::Visualiser() : pimpl(new VisualiserImpl) {};

	Visualiser::Visualiser(std::string name, int width, int height) :
		pimpl(new VisualiserImpl(name, width, height)) {};

	Visualiser::~Visualiser() {
	}

	void Visualiser::addPointCloud(std::string cloudName, PointCloud::Ptr cloud) {
		pimpl->addPointCloud(cloudName, cloud);
	}

	void Visualiser::spin() {
		pimpl->spin();
	}
}