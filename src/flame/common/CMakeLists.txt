file(GLOB_RECURSE FLAME_COMMON_HEADER_LIST "*.h*")
file(GLOB_RECURSE FLAME_COMMON_SOURCE_LIST "*.c*")

group_source("${FLAME_COMMON_HEADER_LIST}" "" "Header")
group_source("${FLAME_COMMON_SOURCE_LIST}" "" "Source")

if (FLAME_BUILD_SHARED_LIBRARY)
	add_library(flame_common SHARED ${FLAME_COMMON_HEADER_LIST} ${FLAME_COMMON_SOURCE_LIST} ${CMAKE_SOURCE_DIR}/src/flame/common_exports.h)
	GENERATE_EXPORT_HEADER(flame_common
		BASE_NAME flame_common 
		EXPORT_MACRO_NAME FLAME_COMMON_EXPORTS
		EXPORT_FILE_NAME ${CMAKE_SOURCE_DIR}/src/flame/common_exports.h
		STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
	)
else()
	add_library(flame_common STATIC ${FLAME_COMMON_HEADER_LIST} ${FLAME_COMMON_SOURCE_LIST})
endif()

target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/rapidxml-1.13")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/gli")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/glm")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/stb")

set_target_properties(flame_common PROPERTIES FOLDER "flame") 