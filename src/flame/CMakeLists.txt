# filesystem
set(FLAME_FILESYSTEM_HEADER_LIST "filesystem.h")
set(FLAME_FILESYSTEM_SOURCE_LIST "filesystem.cpp")

group_source("${FLAME_FILESYSTEM_HEADER_LIST}" "" "Header")
group_source("${FLAME_FILESYSTEM_SOURCE_LIST}" "" "Source")

add_library(flame_filesystem SHARED ${FLAME_FILESYSTEM_HEADER_LIST} ${FLAME_FILESYSTEM_SOURCE_LIST})

target_compile_definitions(flame_filesystem PRIVATE TK_EXPORTS)

target_include_directories(flame_filesystem PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_filesystem PUBLIC "${CMAKE_SOURCE_DIR}/ext/rapidxml-1.13")
target_include_directories(flame_filesystem PUBLIC "${CMAKE_SOURCE_DIR}/ext/glm")

set_target_properties(flame_filesystem PROPERTIES FOLDER "flame")

# math
set(FLAME_MATH_HEADER_LIST "math.h")
set(FLAME_MATH_SOURCE_LIST "math.cpp")

group_source("${FLAME_MATH_HEADER_LIST}" "" "Header")
group_source("${FLAME_MATH_SOURCE_LIST}" "" "Source")

add_library(flame_math SHARED ${FLAME_MATH_HEADER_LIST} ${FLAME_MATH_SOURCE_LIST})

target_compile_definitions(flame_math PRIVATE TK_EXPORTS)

target_include_directories(flame_math PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_math PUBLIC "${CMAKE_SOURCE_DIR}/ext/glm")

set_target_properties(flame_math PROPERTIES FOLDER "flame")

# image
set(FLAME_IMAGE_HEADER_LIST "image.h")
set(FLAME_IMAGE_SOURCE_LIST "image.cpp")

group_source("${FLAME_IMAGE_HEADER_LIST}" "" "Header")
group_source("${FLAME_IMAGE_SOURCE_LIST}" "" "Source")

add_library(flame_image SHARED ${FLAME_IMAGE_HEADER_LIST} ${FLAME_IMAGE_SOURCE_LIST})

target_compile_definitions(flame_image PRIVATE TK_EXPORTS)

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

target_compile_definitions(flame_system PRIVATE TK_EXPORTS)

target_include_directories(flame_system PUBLIC "${CMAKE_SOURCE_DIR}/src")

set_target_properties(flame_system PROPERTIES FOLDER "flame")

set(FLAME_SURFACE_HEADER_LIST "surface.h")
set(FLAME_SURFACE_SOURCE_LIST "surface.cpp")

group_source("${FLAME_SURFACE_HEADER_LIST}" "" "Header")
group_source("${FLAME_SURFACE_SOURCE_LIST}" "" "Source")

add_library(flame_surface SHARED ${FLAME_SURFACE_HEADER_LIST} ${FLAME_SURFACE_SOURCE_LIST})

target_include_directories(flame_surface PUBLIC "${CMAKE_SOURCE_DIR}/src")

target_link_libraries(flame_surface flame_filesystem)
target_link_libraries(flame_surface flame_math)
target_link_libraries(flame_surface flame_system)
target_link_libraries(flame_surface flame_image)

set_target_properties(flame_surface PROPERTIES FOLDER "flame") 

add_subdirectory(engine)