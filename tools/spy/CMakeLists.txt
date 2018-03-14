project(spy)

file(GLOB_RECURSE SPY_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SPY_SOURCE_LIST "src/*.c*")

GROUP_SRC("${SPY_HEADER_LIST}" "/src" "Header")
GROUP_SRC("${SPY_SOURCE_LIST}" "/src" "Source")

add_executable(spy ${SPY_HEADER_LIST} ${SPY_SOURCE_LIST})

target_link_libraries(spy flame_engine)

set_target_properties(spy PROPERTIES FOLDER "tools") 
set_target_properties(spy PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")