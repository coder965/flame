project(code_editor)

file(GLOB_RECURSE EDITOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE EDITOR_SOURCE_LIST "src/*.c*")

GROUP_SRC("${EDITOR_HEADER_LIST}" "/src" "Header")
GROUP_SRC("${EDITOR_SOURCE_LIST}" "/src" "Source")

add_executable(code_editor ${EDITOR_HEADER_LIST} ${EDITOR_SOURCE_LIST})

target_link_libraries(code_editor flame_engine)

set_target_properties(code_editor PROPERTIES FOLDER "tools") 
set_target_properties(code_editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")