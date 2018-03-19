include(../../cmake/utils.cmake)

set_output_dir(${CMAKE_SOURCE_DIR}/../../bin)

set(FREETYPE_LIBRARY "${CMAKE_SOURCE_DIR}/../../ext/msdfgen/lib/freetype")
set(FREETYPE_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/../../ext/msdfgen/lib/include")
add_subdirectory(${CMAKE_SOURCE_DIR}/../../ext/msdfgen build)

project(sdf_generator)

file(GLOB_RECURSE SDF_GENERATOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SDF_GENERATOR_SOURCE_LIST "src/*.c*")

group_source("${SDF_GENERATOR_HEADER_LIST}" "/src" "Header")
group_source("${SDF_GENERATOR_SOURCE_LIST}" "/src" "Source")

add_executable(sdf_generator ${SDF_GENERATOR_HEADER_LIST} ${SDF_GENERATOR_SOURCE_LIST})

target_include_directories(sdf_generator PUBLIC "${CMAKE_SOURCE_DIR}/../../ext/msdfgen")

target_link_libraries(sdf_generator lib_msdfgen)

set_target_properties(sdf_generator PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/../../bin")