file(GLOB_RECURSE FLAME_ENGINE_HEADER_LIST "*.h*")
file(GLOB_RECURSE FLAME_ENGINE_SOURCE_LIST "*.c*")

GROUP_SRC("${FLAME_ENGINE_HEADER_LIST}" "" "Header")
GROUP_SRC("${FLAME_ENGINE_SOURCE_LIST}" "" "Source")

if (TK_LIBRARY_SHARED)
	add_library(flame_engine SHARED ${FLAME_ENGINE_HEADER_LIST} ${FLAME_ENGINE_SOURCE_LIST} ${CMAKE_SOURCE_DIR}/src/flame/engine_exports.h)
	GENERATE_EXPORT_HEADER(flame_engine
		BASE_NAME flame_engine 
		EXPORT_MACRO_NAME FLAME_ENGINE_EXPORTS
		EXPORT_FILE_NAME ${CMAKE_SOURCE_DIR}/src/flame/engine_exports.h
		STATIC_DEFINE SHARED_EXPORTS_BUILT_AS_STATIC
	)
else()
	add_library(flame_engine STATIC ${FLAME_ENGINE_HEADER_LIST} ${FLAME_ENGINE_SOURCE_LIST})
endif()


target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/vulkan")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/IconFontCppHeaders")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Include")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/include")
target_include_directories(flame_engine PUBLIC "${CMAKE_SOURCE_DIR}/ext/msdfgen")

target_link_libraries(flame_engine flame_common)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/${FLAME_EXT_BIN_DIR}/vulkan-1)
target_link_libraries(flame_engine imgui)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win32/PhysX3DEBUG_x86)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win32/PhysX3CommonDEBUG_x86)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win32/PhysX3ExtensionsDEBUG)
target_link_libraries(flame_engine ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/lib/vc15win32/PxFoundationDEBUG_x86)
target_link_libraries(flame_engine spirv-cross-core)
target_link_libraries(flame_engine spirv-cross-cpp)
target_link_libraries(flame_engine spirv-cross-glsl)
target_link_libraries(flame_engine lib_msdfgen)

add_custom_target(copy_dlls_for_flame_engine 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win32/PhysX3CommonDEBUG_x86.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win32/PhysX3DEBUG_x86.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/bin/vc15win32/PxFoundationDEBUG_x86.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/msdfgen/bin/freetype6.dll ${CMAKE_CURRENT_SOURCE_DIR}/bin
)

add_dependencies(flame_engine copy_dlls_for_flame_engine)

set_target_properties(flame_engine PROPERTIES FOLDER "flame") 
