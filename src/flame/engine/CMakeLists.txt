file(GLOB_RECURSE FLAME_ENGINE_HEADER_LIST "*.h*")
file(GLOB_RECURSE FLAME_ENGINE_SOURCE_LIST "*.c*")

group_source("${FLAME_ENGINE_HEADER_LIST}" "" "Header")
group_source("${FLAME_ENGINE_SOURCE_LIST}" "" "Source")

if (FLAME_BUILD_SHARED_LIBRARY)
	add_library(flame_engine SHARED ${FLAME_ENGINE_HEADER_LIST} ${FLAME_ENGINE_SOURCE_LIST})
else()
	add_library(flame_engine STATIC ${FLAME_ENGINE_HEADER_LIST} ${FLAME_ENGINE_SOURCE_LIST})
endif()


target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/vulkan")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/IconFontCppHeaders")
if (FLAME_ENABLE_PHYSICS)
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Include")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/include")
endif()

target_link_libraries(flame_engine flame_filesystem)
target_link_libraries(flame_engine flame_math)
target_link_libraries(flame_engine flame_system)
target_link_libraries(flame_engine flame_image)
target_link_libraries(flame_engine flame_surface)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/bin/x${FLAME_SYS_NAME}/vulkan-1.lib)
target_link_libraries(flame_engine imgui)
if (FLAME_ENABLE_PHYSICS)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3DEBUG_x${FLAME_SYS_NAME})
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3CommonDEBUG_x${FLAME_SYS_NAME})
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3ExtensionsDEBUG)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/lib/vc15win${FLAME_SYS_BIT}/PxFoundationDEBUG_x${FLAME_SYS_NAME})
endif()
target_link_libraries(flame_engine spirv-cross-core)
target_link_libraries(flame_engine spirv-cross-cpp)
target_link_libraries(flame_engine spirv-cross-glsl)

add_custom_target(copy_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/msdfgen/bin/freetype6.dll ${CMAKE_SOURCE_DIR}/bin
)
add_dependencies(flame_engine copy_dlls)

if (FLAME_ENABLE_PHYSICS)
	add_custom_target(copy_physx_dlls 
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win${FLAME_SYS_BIT}/PhysX3CommonDEBUG_x${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win${FLAME_SYS_BIT}/PhysX3DEBUG_x${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/bin/vc15win${FLAME_SYS_BIT}/PxFoundationDEBUG_x${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
	)
	add_dependencies(flame_engine copy_physx_dlls)
endif()

configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

set_target_properties(flame_engine PROPERTIES FOLDER "flame") 
