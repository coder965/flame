set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0)

if (WIN32)
	function(GROUP_SRC SRCS FILE_PREFIX GROUP_PREFIX)
		foreach(FILE ${SRCS}) 
			get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
			string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}${FILE_PREFIX}" "" GROUP "${PARENT_DIR}")
			string(REPLACE "/" "\\" GROUP "${GROUP}")
			source_group("${GROUP_PREFIX}${GROUP}" FILES "${FILE}")
		endforeach()
	endfunction()
endif()

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
	string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
endforeach()

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