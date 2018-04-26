project(graphics_test)

file(GLOB_RECURSE GRAPHICS_TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE GRAPHICS_TEST_SOURCE_LIST "src/*.c*")

group_source("${GRAPHICS_TEST_HEADER_LIST}" "/src" "Header")
group_source("${GRAPHICS_TEST_SOURCE_LIST}" "/src" "Source")

add_executable(graphics_test ${GRAPHICS_TEST_HEADER_LIST} ${GRAPHICS_TEST_SOURCE_LIST})

target_link_libraries(graphics_test flame_surface)
target_link_libraries(graphics_test flame_graphics)
target_link_libraries(graphics_test flame_model)

set_target_properties(graphics_test PROPERTIES FOLDER "tests") 
set_target_properties(graphics_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")