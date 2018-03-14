project(playground)

file(GLOB_RECURSE PLAYGROUND_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE PLAYGROUND_SOURCE_LIST "src/*.c*")

GROUP_SRC("${PLAYGROUND_HEADER_LIST}" "/src" "Header")
GROUP_SRC("${PLAYGROUND_SOURCE_LIST}" "/src" "Source")

add_executable(playground ${PLAYGROUND_HEADER_LIST} ${PLAYGROUND_SOURCE_LIST})

target_link_libraries(playground flame_engine)

set_target_properties(playground PROPERTIES FOLDER "tools") 
set_target_properties(playground PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")