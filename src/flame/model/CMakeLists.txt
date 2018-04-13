file(GLOB FLAME_MODEL_HEADER_LIST "*.h*")
file(GLOB FLAME_MODEL_SOURCE_LIST "*.c*")

group_source("${FLAME_MODEL_HEADER_LIST}" "" "Header")
group_source("${FLAME_MODEL_SOURCE_LIST}" "" "Source")

add_library(flame_model SHARED ${FLAME_MODEL_HEADER_LIST} ${FLAME_MODEL_SOURCE_LIST})

#target_compile_options(flame_model PRIVATE "/MT$<$<STREQUAL:$<CONFIGURATION>,Debug>:d>")

target_compile_definitions(flame_model PRIVATE _FLAME_MODEL_EXPORTS)

target_include_directories(flame_model PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_model PUBLIC "${CMAKE_SOURCE_DIR}/ext/assimp/include")

target_link_libraries(flame_model flame_filesystem)
target_link_libraries(flame_model flame_math)
target_link_libraries(flame_model assimp)

set_target_properties(flame_model PROPERTIES FOLDER "flame") 
