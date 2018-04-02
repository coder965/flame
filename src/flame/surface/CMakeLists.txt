file(GLOB_RECURSE FLAME_SURFACE_HEADER_LIST "*.h*")
file(GLOB_RECURSE FLAME_SURFACE_SOURCE_LIST "*.c*")

group_source("${FLAME_SURFACE_HEADER_LIST}" "" "Header")
group_source("${FLAME_SURFACE_SOURCE_LIST}" "" "Source")

add_library(flame_surface SHARED ${FLAME_SURFACE_HEADER_LIST} ${FLAME_SURFACE_SOURCE_LIST})

target_include_directories(flame_surface PUBLIC "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(flame_surface flame_filesystem)
target_link_libraries(flame_surface flame_math)
target_link_libraries(flame_surface flame_system)
target_link_libraries(flame_surface flame_image)

set_target_properties(flame_surface PROPERTIES FOLDER "flame") 
