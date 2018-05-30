cmake_minimum_required(VERSION 3.4)

set(FLAME_GRAPHICS "VULKAN" CACHE STRING "FLAME_GRAPHICS_API")
option(FLAME_BUILD_SHARED_LIBRARY "FLAME_BUILD_SHARED_LIBRARY" OFF)
option(FLAME_ENABLE_PHYSICS "FLAME_ENABLE_PHYSICS" OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0)

include(cmake/utils.cmake)

set_output_dir("${CMAKE_SOURCE_DIR}/bin")

project(flame)

if(CMAKE_SIZEOF_VOID_P MATCHES "8")
	set(FLAME_SYS_NAME "x64")
	set(FLAME_SYS_BIT "64")
else()
	set(FLAME_SYS_NAME "x86")
	set(FLAME_SYS_BIT "32")
endif()

add_subdirectory(ext)

if (FLAME_GRAPHICS MATCHES "OPENGL_3_2")
	set(FLAME_GRAPHICS_OPENGL_3_2 ON)
else()
	set(FLAME_GRAPHICS_VULKAN ON)
endif()

configure_file(src/flame/config.in ${CMAKE_SOURCE_DIR}/src/flame/config.h)

add_subdirectory(src/flame)
add_subdirectory(tests)
add_subdirectory(games)
add_subdirectory(tools)