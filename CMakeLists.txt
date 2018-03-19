set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0)

include(cmake/utils.cmake)

set_output_dir("${CMAKE_SOURCE_DIR}/bin")

project(flame)

add_subdirectory(ext)

if(CMAKE_CL_64)
	set(FLAME_SYS_NAME "64")
	set(FLAME_SYS_BIT "64")
else()
	set(FLAME_SYS_NAME "86")
	set(FLAME_SYS_BIT "32")
endif()

include(GenerateExportHeader)

option(FLAME_BUILD_SHARED_LIBRARY "FLAME_BUILD_SHARED_LIBRARY" OFF)
option(FLAME_ENABLE_PHYSICS "FLAME_ENABLE_PHYSICS" OFF)

if (FLAME_ENABLE_PHYSICS)
	set(FLAME_ENABLE_PHYSICS_VAR 1)
else()
	set(FLAME_ENABLE_PHYSICS_VAR 0)
endif()

add_subdirectory(src)

add_subdirectory(tools)