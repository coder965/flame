file(GLOB_RECURSE UI_TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE UI_TEST_SOURCE_LIST "src/*.c*")

group_source("${UI_TEST_HEADER_LIST}" "/src" "Header")
group_source("${UI_TEST_SOURCE_LIST}" "/src" "Source")

add_executable(UI_test ${UI_TEST_HEADER_LIST} ${UI_TEST_SOURCE_LIST})

target_link_libraries(UI_test flame_surface)
target_link_libraries(UI_test flame_system)
target_link_libraries(UI_test flame_filesystem)
target_link_libraries(UI_test flame_graphics)
target_link_libraries(UI_test flame_UI)

set_target_properties(UI_test PROPERTIES FOLDER "tests") 
set_target_properties(UI_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
