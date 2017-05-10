cmake_minimum_required(VERSION 3.5)
project(CloudLibrary)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-std=c++14)

find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
	MESSAGE("Could not find OpenGL")
	RETURN()
endif()
include_directories(${OPENGL_INCLUDE_DIR})

find_package(GLEW REQUIRED)
if(NOT GLEW_FOUND)
	MESSAGE("Could not find GLEW.")
	RETURN()
endif()
include_directories($(GLEW_INCLUDE_DIRS))

find_package(glfw3)
if(NOT glfw3_FOUND)
	MESSAGE("Could not find GLFW3.")
	RETURN()
endif()
include_directories(${GLFW_INCLUDE_DIR})

find_package(glm)
if(NOT glm_FOUND)
	MESSAGE("Could not find GLM.")
	RETURN()
endif()
include_directories(${GLM_INCLUDE_DIRS})

set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost COMPONENTS system filesystem REQUIRED)
if( NOT Boost_FOUND)
	MESSAGE("Could not find BOOST.")
endif()
include_directories(${Boost_INCLUDE_DIRS})


include_directories(vendor)
include_directories(include)

set(CL_FILES 
	src/visualiser.cpp
	src/visualiser_impl.hpp
	include/io.hpp
	include/point_cloud.hpp
	include/visualiser.hpp)



add_library(CloudLibrary ${CL_FILES})
target_link_libraries(CloudLibrary glfw ${GLFW_LIBRARIES} ${GLEW_LIBRARY} ${OPENGL_gl_LIBRARY} ${Boost_LIBRARIES})

set(VIS_TEST_FILES tests/visualiser_test.cpp)
add_executable(VisualiserTest ${VIS_TEST_FILES})
target_link_libraries(VisualiserTest CloudLibrary)

set(POINT_CLOUD_UNIT_TEST_FILES tests/point_cloud_unit_test.cpp)
add_executable(PointCloudUnitTest ${POINT_CLOUD_UNIT_TEST_FILES})
