project(terrain_test)

file(GLOB_RECURSE TERRAIN_TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE TERRAIN_TEST_SOURCE_LIST "src/*.c*")

group_source("${TERRAIN_TEST_HEADER_LIST}" "/src" "Header")
group_source("${TERRAIN_TEST_SOURCE_LIST}" "/src" "Source")

add_executable(terrain_test ${TERRAIN_TEST_HEADER_LIST} ${TERRAIN_TEST_SOURCE_LIST})

target_link_libraries(terrain_test flame_surface)
target_link_libraries(terrain_test flame_graphics)
target_link_libraries(terrain_test flame_model)

set_target_properties(terrain_test PROPERTIES FOLDER "tests") 
set_target_properties(terrain_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")