project(skeleton_test)

file(GLOB_RECURSE SKELETON_TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SKELETON_TEST_SOURCE_LIST "src/*.c*")

group_source("${SKELETON_TEST_HEADER_LIST}" "/src" "Header")
group_source("${SKELETON_TEST_SOURCE_LIST}" "/src" "Source")

add_executable(skeleton_test ${SKELETON_TEST_HEADER_LIST} ${SKELETON_TEST_SOURCE_LIST})

target_link_libraries(skeleton_test flame_surface)
target_link_libraries(skeleton_test flame_graphics)
target_link_libraries(skeleton_test flame_model)

set_target_properties(skeleton_test PROPERTIES FOLDER "tools") 
set_target_properties(skeleton_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")