project(physics_test)

file(GLOB_RECURSE PHYSICS_TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE PHYSICS_TEST_SOURCE_LIST "src/*.c*")

group_source("${PHYSICS_TEST_HEADER_LIST}" "/src" "Header")
group_source("${PHYSICS_TEST_SOURCE_LIST}" "/src" "Source")

add_executable(physics_test ${PHYSICS_TEST_HEADER_LIST} ${PHYSICS_TEST_SOURCE_LIST})

target_link_libraries(physics_test flame_surface)
target_link_libraries(physics_test flame_graphics)
target_link_libraries(physics_test flame_model)
target_link_libraries(physics_test flame_physics)
target_link_libraries(physics_test flame_UI)

set_target_properties(physics_test PROPERTIES FOLDER "tests") 
set_target_properties(physics_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")