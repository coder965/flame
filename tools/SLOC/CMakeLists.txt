project(SLOC)

file(GLOB_RECURSE SLOC_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SLOC_SOURCE_LIST "src/*.c*")

GROUP_SRC("${SLOC_HEADER_LIST}" "/src" "Header")
GROUP_SRC("${SLOC_SOURCE_LIST}" "/src" "Source")

add_executable(SLOC ${SLOC_HEADER_LIST} ${SLOC_SOURCE_LIST})

target_link_libraries(SLOC flame_common)

set_target_properties(SLOC PROPERTIES FOLDER "tools") 