project(tk_test)

file(GLOB_RECURSE TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE TEST_SOURCE_LIST "src/*.c*")

group_source("${TEST_HEADER_LIST}" "/src" "Header")
group_source("${TEST_SOURCE_LIST}" "/src" "Source")

add_executable(tk_test ${TEST_HEADER_LIST} ${TEST_SOURCE_LIST})

target_link_libraries(tk_test flame_surface)
target_link_libraries(tk_test flame_graphics)
target_link_libraries(tk_test flame_model)

set_target_properties(tk_test PROPERTIES FOLDER "tools") 
set_target_properties(tk_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")