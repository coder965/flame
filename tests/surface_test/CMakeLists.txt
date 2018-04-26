project(surface_test)

file(GLOB_RECURSE SURFACE_TEST_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SURFACE_TEST_SOURCE_LIST "src/*.c*")

group_source("${SURFACE_TEST_HEADER_LIST}" "/src" "Header")
group_source("${SURFACE_TEST_SOURCE_LIST}" "/src" "Source")

add_executable(surface_test ${SURFACE_TEST_HEADER_LIST} ${SURFACE_TEST_SOURCE_LIST})

target_link_libraries(surface_test flame_surface)

set_target_properties(surface_test PROPERTIES FOLDER "tests") 
set_target_properties(surface_test PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")