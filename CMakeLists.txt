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

foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin)
endforeach()

project(flame)

file(GLOB_RECURSE FLAME_HEADER_LIST "src/flame/*.h*")
file(GLOB_RECURSE FLAME_SOURCE_LIST "src/flame/*.c*")

GROUP_SRC("${FLAME_HEADER_LIST}" "src/flame" "Header")
GROUP_SRC("${FLAME_SOURCE_LIST}" "src/flame" "Source")

add_library(flame  ${FLAME_HEADER_LIST} ${FLAME_SOURCE_LIST})

target_include_directories(flame PUBLIC "src")

add_subdirectory(ext)
add_subdirectory(tools)

if(CMAKE_CL_64)
set(FLAME_EXT_BIN_DIR "ext/bin/x64")
else()
set(FLAME_EXT_BIN_DIR "ext/bin/x86")
endif()

target_include_directories(flame PUBLIC "ext/vulkan")
target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/${FLAME_EXT_BIN_DIR}/vulkan-1)

target_include_directories(flame PUBLIC "ext/FreeImage")
target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/${FLAME_EXT_BIN_DIR}/FreeImage)

target_include_directories(flame PUBLIC "ext/gli")

target_include_directories(flame PUBLIC "ext/glm")

target_include_directories(flame PUBLIC "ext/IconFontCppHeaders")

target_link_libraries(flame imgui)

target_include_directories(flame PUBLIC "ext/Physx-3.4/PhysX_3.4/Include")
target_include_directories(flame PUBLIC "ext/Physx-3.4/PxShared/include")
target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win32/PhysX3DEBUG_x86)
target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win32/PhysX3CommonDEBUG_x86)
target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win32/PhysX3ExtensionsDEBUG)
target_link_libraries(flame ${CMAKE_CURRENT_SOURCE_DIR}/ext/Physx-3.4/PxShared/lib/vc15win32/PxFoundationDEBUG_x86)

target_include_directories(flame PUBLIC "ext/rapidxml-1.13")

target_link_libraries(flame spirv-cross-core)
target_link_libraries(flame spirv-cross-cpp)
target_link_libraries(flame spirv-cross-glsl)

target_link_libraries(flame lib_msdfgen)
target_include_directories(flame PUBLIC "ext/msdfgen")

add_custom_target(copy_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win32/PhysX3CommonDEBUG_x86.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win32/PhysX3DEBUG_x86.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/ext/Physx-3.4/PxShared/bin/vc15win32/PxFoundationDEBUG_x86.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/ext/bin/x86/FreeImage.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/ext/msdfgen/bin/freetype6.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

add_dependencies(flame copy_dlls)