# filesystem
set(FLAME_FILESYSTEM_HEADER_LIST "filesystem.h")
set(FLAME_FILESYSTEM_SOURCE_LIST "filesystem.cpp")

group_source("${FLAME_FILESYSTEM_HEADER_LIST}" "" "Header")
group_source("${FLAME_FILESYSTEM_SOURCE_LIST}" "" "Source")

add_library(flame_filesystem SHARED ${FLAME_FILESYSTEM_HEADER_LIST} ${FLAME_FILESYSTEM_SOURCE_LIST})

target_compile_definitions(flame_filesystem PRIVATE _FLAME_FILESYSTEM_EXPORTS)

target_include_directories(flame_filesystem PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_filesystem PUBLIC "${CMAKE_SOURCE_DIR}/ext/rapidxml-1.13")
target_include_directories(flame_filesystem PUBLIC "${CMAKE_SOURCE_DIR}/ext/glm")

set_target_properties(flame_filesystem PROPERTIES FOLDER "flame")

# image
set(FLAME_IMAGE_HEADER_LIST "image.h")
set(FLAME_IMAGE_SOURCE_LIST "image.cpp")

group_source("${FLAME_IMAGE_HEADER_LIST}" "" "Header")
group_source("${FLAME_IMAGE_SOURCE_LIST}" "" "Source")

add_library(flame_image SHARED ${FLAME_IMAGE_HEADER_LIST} ${FLAME_IMAGE_SOURCE_LIST})

target_compile_definitions(flame_image PRIVATE _FLAME_IMAGE_EXPORTS)

target_include_directories(flame_image PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_image PUBLIC "${CMAKE_SOURCE_DIR}/ext/gli")
target_include_directories(flame_image PUBLIC "${CMAKE_SOURCE_DIR}/ext/stb")

set_target_properties(flame_image PROPERTIES FOLDER "flame")

# system
set(FLAME_SYSTEM_HEADER_LIST "system.h")
set(FLAME_SYSTEM_SOURCE_LIST "system.cpp")

group_source("${FLAME_SYSTEM_HEADER_LIST}" "" "Header")
group_source("${FLAME_SYSTEM_SOURCE_LIST}" "" "Source")

add_library(flame_system SHARED ${FLAME_SYSTEM_HEADER_LIST} ${FLAME_SYSTEM_SOURCE_LIST})

target_compile_definitions(flame_system PRIVATE _FLAME_SYSTEM_EXPORTS)

target_include_directories(flame_system PUBLIC "${CMAKE_SOURCE_DIR}/src")

set_target_properties(flame_system PROPERTIES FOLDER "flame")

set(FLAME_SURFACE_HEADER_LIST "surface.h")
set(FLAME_SURFACE_SOURCE_LIST "surface.cpp")

group_source("${FLAME_SURFACE_HEADER_LIST}" "" "Header")
group_source("${FLAME_SURFACE_SOURCE_LIST}" "" "Source")

add_library(flame_surface SHARED ${FLAME_SURFACE_HEADER_LIST} ${FLAME_SURFACE_SOURCE_LIST})

target_compile_definitions(flame_surface PRIVATE _FLAME_SURFACE_EXPORTS)

target_include_directories(flame_surface PUBLIC "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(flame_surface flame_filesystem)
target_link_libraries(flame_surface flame_system)
target_link_libraries(flame_surface flame_image)

set_target_properties(flame_surface PROPERTIES FOLDER "flame") 

add_subdirectory(shader)
add_subdirectory(graphics)
add_subdirectory(UI)
add_subdirectory(model)
if (FLAME_ENABLE_PHYSICS)
add_subdirectory(physics)
endif()
#add_subdirectory(engine)