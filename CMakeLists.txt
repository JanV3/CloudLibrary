cmake_minimum_required(VERSION 3.7)
project(CloudLibrary)

set(CMAKE_CXX_STANDARD 14)
add_definitions(-std=c++1z)

include_directories(vendor)
include_directories(include)
find_package(glfw3)
find_package(OpenGL REQUIRED)
find_package(Boost REQUIRED system filesystem)
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIR})

set(SOURCE_FILES main.cpp include/cl.hpp include/visualiser.h include/camera.h)
set(TEST_FILES tests/cl_tests.cpp include/cl.hpp)

add_executable(Main ${SOURCE_FILES})
target_link_libraries(Main glfw ${GLFW_LIBRARIES} ${OPENGL_gl_LIBRARY} ${Boost_LIBRARIES})
add_executable(Tests ${TEST_FILES})
