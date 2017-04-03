cmake_minimum_required(VERSION 3.6)
project(CubeCraft)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIRS})
	link_libraries(${OPENGL_LIBRARIES})
endif()

find_package(GLEW REQUIRED)
if (GLEW_FOUND)
	include_directories(${GLEW_INCLUDE_DIRS})
	link_libraries(${GLEW_LIBRARIES})
endif()

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2.4 REQUIRED system window graphics)
if(SFML_FOUND)
	include_directories(${SFML_INCLUDE_DIR})
	link_libraries(${SFML_LIBRARIES})
endif()


find_package(Threads REQUIRED)
if(THREADS_FOUND)
	link_libraries(${CMAKE_THREAD_LIBS_INIT})
endif()

file(GLOB SOURCE_FILES src/*.cpp src/*/*.cpp)

add_executable(CubeCraft ${SOURCE_FILES})
