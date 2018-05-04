file(GLOB FLAME_SHADER_HEADER_LIST "*.h*")
file(GLOB FLAME_SHADER_SOURCE_LIST "*.c*")

group_source("${FLAME_SHADER_HEADER_LIST}" "" "Header")
group_source("${FLAME_SHADER_SOURCE_LIST}" "" "Source")

add_library(flame_shader SHARED ${FLAME_SHADER_HEADER_LIST} ${FLAME_SHADER_SOURCE_LIST})

target_compile_definitions(flame_shader PRIVATE _FLAME_SHADER_EXPORTS)

target_include_directories(flame_shader PUBLIC "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(flame_shader flame_filesystem)
target_link_libraries(flame_shader flame_system)
target_link_libraries(flame_shader spirv-cross-core)
target_link_libraries(flame_shader spirv-cross-cpp)
target_link_libraries(flame_shader spirv-cross-glsl)

set_target_properties(flame_shader PROPERTIES FOLDER "flame") 
