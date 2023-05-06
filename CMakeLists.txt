cmake_minimum_required(VERSION 3.26)

project(spokes)

add_subdirectory(external)

add_executable(spokes WIN32
	source/main.cpp

	include/spokes.hpp
)

target_link_libraries(spokes
	PUBLIC cinder
)

target_include_directories(spokes PUBLIC
	include
	external/cinder/include
)

target_compile_options(spokes
	PRIVATE "/std:c++20"
)