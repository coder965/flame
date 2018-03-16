project(sdf_generator)

file(GLOB_RECURSE SDF_GENERATOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SDF_GENERATOR_SOURCE_LIST "src/*.c*")

GROUP_SRC("${SDF_GENERATOR_HEADER_LIST}" "/src" "Header")
GROUP_SRC("${SDF_GENERATOR_SOURCE_LIST}" "/src" "Source")

add_executable(sdf_generator ${SDF_GENERATOR_HEADER_LIST} ${SDF_GENERATOR_SOURCE_LIST})

target_include_directories(sdf_generator PUBLIC "${CMAKE_SOURCE_DIR}/ext/msdfgen")

target_link_libraries(sdf_generator flame_common)
target_link_libraries(sdf_generator lib_msdfgen)

set_target_properties(sdf_generator PROPERTIES FOLDER "tools") 
set_target_properties(sdf_generator PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")