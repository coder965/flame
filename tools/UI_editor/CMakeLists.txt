file(GLOB_RECURSE UI_EDITOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE UI_EDITOR_SOURCE_LIST "src/*.c*")

group_source("${UI_EDITOR_HEADER_LIST}" "/src" "Header")
group_source("${UI_EDITOR_SOURCE_LIST}" "/src" "Source")

add_executable(UI_editor ${UI_EDITOR_HEADER_LIST} ${UI_EDITOR_SOURCE_LIST})

target_link_libraries(UI_editor flame_surface)
target_link_libraries(UI_editor flame_system)
target_link_libraries(UI_editor flame_filesystem)
target_link_libraries(UI_editor flame_graphics)
target_link_libraries(UI_editor flame_UI)

set_target_properties(UI_editor PROPERTIES FOLDER "tools") 
set_target_properties(UI_editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
