project(pin)

file(GLOB_RECURSE PIN_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE PIN_SOURCE_LIST "src/*.c*")

group_source("${PIN_HEADER_LIST}" "/src" "Header")
group_source("${PIN_SOURCE_LIST}" "/src" "Source")

add_executable(pin ${PIN_HEADER_LIST} ${PIN_SOURCE_LIST})

target_link_libraries(pin flame_system)

set_target_properties(pin PROPERTIES FOLDER "tools") 
set_target_properties(pin PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")