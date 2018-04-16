project(tk_pin)

file(GLOB_RECURSE TK_PIN_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE TK_PIN_SOURCE_LIST "src/*.c*")

group_source("${TK_PIN_HEADER_LIST}" "/src" "Header")
group_source("${TK_PIN_SOURCE_LIST}" "/src" "Source")

add_executable(tk_pin ${TK_PIN_HEADER_LIST} ${TK_PIN_SOURCE_LIST})

target_link_libraries(tk_pin flame_system)

set_target_properties(tk_pin PROPERTIES FOLDER "tools") 
set_target_properties(tk_pin PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")