file(GLOB_RECURSE EFFECT_EDITOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE EFFECT_EDITOR_SOURCE_LIST "src/*.c*")

group_source("${EFFECT_EDITOR_HEADER_LIST}" "/src" "Header")
group_source("${EFFECT_EDITOR_SOURCE_LIST}" "/src" "Source")

add_executable(effect_editor ${EFFECT_EDITOR_HEADER_LIST} ${EFFECT_EDITOR_SOURCE_LIST})

target_link_libraries(effect_editor flame_surface)
target_link_libraries(effect_editor flame_system)
target_link_libraries(effect_editor flame_filesystem)
target_link_libraries(effect_editor flame_graphics)
target_link_libraries(effect_editor flame_UI)

set_target_properties(effect_editor PROPERTIES FOLDER "tools") 
set_target_properties(effect_editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
