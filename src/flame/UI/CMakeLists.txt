file(GLOB FLAME_UI_HEADER_LIST "*.h*")
file(GLOB FLAME_UI_SOURCE_LIST "*.c*")

group_source("${FLAME_UI_HEADER_LIST}" "" "Header")
group_source("${FLAME_UI_SOURCE_LIST}" "" "Source")

add_library(flame_UI SHARED ${FLAME_UI_HEADER_LIST} ${FLAME_UI_SOURCE_LIST})

target_compile_definitions(flame_UI PRIVATE _FLAME_UI_EXPORTS)

target_include_directories(flame_UI PUBLIC "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(flame_UI imgui)
target_link_libraries(flame_UI flame_graphics)
target_link_libraries(flame_UI flame_system)

set_target_properties(flame_UI PROPERTIES FOLDER "flame") 
