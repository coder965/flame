project(SLOC)

file(GLOB_RECURSE EDITOR_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE EDITOR_SOURCE_LIST "src/*.c*")

GROUP_SRC("${EDITOR_HEADER_LIST}" "src" "Header")
GROUP_SRC("${EDITOR_SOURCE_LIST}" "src" "Source")

add_executable(SLOC ${EDITOR_HEADER_LIST} ${EDITOR_SOURCE_LIST})

set_target_properties(SLOC PROPERTIES FOLDER "tools") 