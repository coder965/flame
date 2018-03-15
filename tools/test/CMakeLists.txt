project(test)

file(GLOB_RECURSE TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE TEST_SOURCE_LIST "src/*.c*")

GROUP_SRC("${TEST_HEADER_LIST}" "/src" "Header")
GROUP_SRC("${TEST_SOURCE_LIST}" "/src" "Source")

add_executable(test ${TEST_HEADER_LIST} ${TEST_SOURCE_LIST})

set_target_properties(test PROPERTIES FOLDER "tools") 
set_target_properties(test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")