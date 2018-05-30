file(GLOB FLAME_GRAPHICS_HEADER_LIST "*.h*")
file(GLOB FLAME_GRAPHICS_SOURCE_LIST "*.c*")

group_source("${FLAME_GRAPHICS_HEADER_LIST}" "" "Header")
group_source("${FLAME_GRAPHICS_SOURCE_LIST}" "" "Source")

add_library(flame_graphics SHARED ${FLAME_GRAPHICS_HEADER_LIST} ${FLAME_GRAPHICS_SOURCE_LIST})

target_compile_definitions(flame_graphics PRIVATE _FLAME_GRAPHICS_EXPORTS)

target_include_directories(flame_graphics PUBLIC "${CMAKE_SOURCE_DIR}/src")
if (FLAME_GRAPHICS_OPENGL_3_2)
	target_include_directories(flame_graphics PUBLIC "${CMAKE_SOURCE_DIR}/ext/glew/include")
else()
	target_include_directories(flame_graphics PUBLIC "$ENV{VK_SDK_PATH}/Include")
endif()

target_link_libraries(flame_graphics flame_filesystem)
target_link_libraries(flame_graphics flame_system)
target_link_libraries(flame_graphics flame_image)
if (FLAME_GRAPHICS_OPENGL_3_2)
	target_link_libraries(flame_graphics opengl32.lib)
	target_link_libraries(flame_graphics ${CMAKE_SOURCE_DIR}/ext/glew/lib/${FLAME_SYS_NAME}/glew32.lib)
else()
	target_link_libraries(flame_graphics $ENV{VK_SDK_PATH}/Lib/vulkan-1.lib)
	target_link_libraries(flame_graphics flame_shader)
endif()

if (FLAME_GRAPHICS_OPENGL_3_2)
	add_custom_target(copy_glew_dlls 
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/glew/bin/${FLAME_SYS_NAME}/glew32.dll ${CMAKE_SOURCE_DIR}/bin
	)
	add_dependencies(flame_graphics copy_glew_dlls)
endif()

set_target_properties(flame_graphics PROPERTIES FOLDER "flame") 
