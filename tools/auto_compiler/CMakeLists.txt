file(GLOB_RECURSE AUTO_COMPILER_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE AUTO_COMPILER_SOURCE_LIST "src/*.c*")

group_source("${AUTO_COMPILER_HEADER_LIST}" "/src" "Header")
group_source("${AUTO_COMPILER_SOURCE_LIST}" "/src" "Source")

add_executable(auto_compiler ${AUTO_COMPILER_HEADER_LIST} ${AUTO_COMPILER_SOURCE_LIST})

target_link_libraries(auto_compiler flame_filesystem)
target_link_libraries(auto_compiler flame_system)

set_target_properties(auto_compiler PROPERTIES FOLDER "tools") 
set_target_properties(auto_compiler PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")