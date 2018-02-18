set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_definitions(-W0)

function(GROUP_SRC SRCS FILE_PREFIX GROUP_PREFIX)
  foreach(FILE ${SRCS}) 
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PREFIX}" "" GROUP "${PARENT_DIR}")
    string(REPLACE "/" "\\" GROUP "${GROUP}")
    set(GROUP "${GROUP_PREFIX}${GROUP}")
    source_group("${GROUP}" FILES "${FILE}")
  endforeach()
endfunction()

project(flame)

file(GLOB_RECURSE FLAME_HEADER_LIST "src/flame/*.h*")
file(GLOB_RECURSE FLAME_SOURCE_LIST "src/flame/*.c*")

GROUP_SRC("${FLAME_HEADER_LIST}" "src/flame" "Header")
GROUP_SRC("${FLAME_SOURCE_LIST}" "src/flame" "Source")

ADD_LIBRARY(flame  ${FLAME_HEADER_LIST} ${FLAME_SOURCE_LIST})

target_include_directories(flame PUBLIC "src")

ADD_SUBDIRECTORY(ext)
ADD_SUBDIRECTORY(tools)

IF(CMAKE_CL_64)
SET(FLAME_EXT_BIN_DIR "ext/bin/x64")
ELSE()
SET(FLAME_EXT_BIN_DIR "ext/bin/x86")
ENDIF()

target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/${FLAME_EXT_BIN_DIR}/vulkan-1)

target_include_directories(flame PUBLIC "ext/FreeImage")
target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/${FLAME_EXT_BIN_DIR}/FreeImage)

target_include_directories(flame PUBLIC "ext/gli")

target_include_directories(flame PUBLIC "ext/glm")

target_include_directories(flame PUBLIC "ext/IconFontCppHeaders")

target_link_libraries(flame imgui)

target_include_directories(flame PUBLIC "ext/Physx-3.4/PhysX_3.4/Include")
target_include_directories(flame PUBLIC "ext/Physx-3.4/PxShared/include")

target_include_directories(flame PUBLIC "ext/rapidxml-1.13")

target_link_libraries(flame spirv-cross-core)
target_link_libraries(flame spirv-cross-cpp)
target_link_libraries(flame spirv-cross-glsl)

target_include_directories(flame PUBLIC "ext/vulkan")