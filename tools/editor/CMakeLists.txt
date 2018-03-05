project(editor)

file(GLOB_RECURSE EDITOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE EDITOR_SOURCE_LIST "src/*.c*")

GROUP_SRC("${EDITOR_HEADER_LIST}" "/src" "Header")
GROUP_SRC("${EDITOR_SOURCE_LIST}" "/src" "Source")

add_executable(editor ${EDITOR_HEADER_LIST} ${EDITOR_SOURCE_LIST})

target_link_libraries(editor flame_engine)

set_target_properties(editor PROPERTIES FOLDER "tools") 
set_target_properties(editor PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")