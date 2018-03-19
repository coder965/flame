project(editor)

file(GLOB_RECURSE EDITOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE EDITOR_SOURCE_LIST "src/*.c*")

group_source("${EDITOR_HEADER_LIST}" "/src" "Header")
group_source("${EDITOR_SOURCE_LIST}" "/src" "Source")

add_executable(editor ${EDITOR_HEADER_LIST} ${EDITOR_SOURCE_LIST})

target_link_libraries(editor flame_engine)

set_target_properties(editor PROPERTIES FOLDER "tools") 
set_target_properties(editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")