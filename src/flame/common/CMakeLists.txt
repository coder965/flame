file(GLOB_RECURSE FLAME_COMMON_HEADER_LIST "*.h*")
file(GLOB_RECURSE FLAME_COMMON_SOURCE_LIST "*.c*")

GROUP_SRC("${FLAME_COMMON_HEADER_LIST}" "" "Header")
GROUP_SRC("${FLAME_COMMON_SOURCE_LIST}" "" "Source")

add_library(flame_common STATIC ${FLAME_COMMON_HEADER_LIST} ${FLAME_COMMON_SOURCE_LIST} ${CMAKE_SOURCE_DIR}/src/flame/common_exports.h)

GENERATE_EXPORT_HEADER(flame_common
    BASE_NAME flame_common 
    EXPORT_MACRO_NAME FLAME_COMMON_EXPORTS
    EXPORT_FILE_NAME ${CMAKE_SOURCE_DIR}/src/flame/common_exports.h
    STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
)

target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/rapidxml-1.13")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/gli")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/glm")
target_include_directories(flame_common PUBLIC "${CMAKE_SOURCE_DIR}/ext/FreeImage")

target_link_libraries(flame_common ${CMAKE_SOURCE_DIR}/${FLAME_EXT_BIN_DIR}/FreeImage)

add_custom_target(copy_dlls_for_flame_common 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/bin/x86/FreeImage.dll ${CMAKE_SOURCE_DIR}/bin
)

add_dependencies(flame_common copy_dlls_for_flame_common)

set_target_properties(flame_common PROPERTIES FOLDER "flame") 