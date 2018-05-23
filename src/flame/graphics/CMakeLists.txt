file(GLOB FLAME_GRAPHICS_HEADER_LIST "*.h*")
file(GLOB FLAME_GRAPHICS_SOURCE_LIST "*.c*")

group_source("${FLAME_GRAPHICS_HEADER_LIST}" "" "Header")
group_source("${FLAME_GRAPHICS_SOURCE_LIST}" "" "Source")

add_library(flame_graphics SHARED ${FLAME_GRAPHICS_HEADER_LIST} ${FLAME_GRAPHICS_SOURCE_LIST})

target_compile_definitions(flame_graphics PRIVATE _FLAME_GRAPHICS_EXPORTS)

target_include_directories(flame_graphics PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_graphics PUBLIC "$ENV{VK_SDK_PATH}/Include")

target_link_libraries(flame_graphics flame_filesystem)
target_link_libraries(flame_graphics flame_system)
target_link_libraries(flame_graphics flame_image)
target_link_libraries(flame_graphics $ENV{VK_SDK_PATH}/Lib/vulkan-1.lib)
target_link_libraries(flame_graphics flame_shader)

set_target_properties(flame_graphics PROPERTIES FOLDER "flame") 
