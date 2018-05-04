file(GLOB FLAME_PHYSICS_HEADER_LIST "*.h*")
file(GLOB FLAME_PHYSICS_SOURCE_LIST "*.c*")

group_source("${FLAME_PHYSICS_HEADER_LIST}" "" "Header")
group_source("${FLAME_PHYSICS_SOURCE_LIST}" "" "Source")

add_library(flame_physics SHARED ${FLAME_PHYSICS_HEADER_LIST} ${FLAME_PHYSICS_SOURCE_LIST})

target_compile_definitions(flame_physics PRIVATE _FLAME_PHYSICS_EXPORTS)

target_include_directories(flame_physics PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(flame_physics PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Include")
target_include_directories(flame_physics PUBLIC "${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/include")

target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3DEBUG_x${FLAME_SYS_NAME})
target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3CommonDEBUG_x${FLAME_SYS_NAME})
target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Lib/vc15win${FLAME_SYS_BIT}/PhysX3ExtensionsDEBUG)
target_link_libraries(flame_physics ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/lib/vc15win${FLAME_SYS_BIT}/PxFoundationDEBUG_x${FLAME_SYS_NAME})

add_custom_target(copy_physx_dlls 
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win${FLAME_SYS_BIT}/PhysX3CommonDEBUG_x${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PhysX_3.4/Bin/vc15win${FLAME_SYS_BIT}/PhysX3DEBUG_x${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_SOURCE_DIR}/ext/Physx-3.4/PxShared/bin/vc15win${FLAME_SYS_BIT}/PxFoundationDEBUG_x${FLAME_SYS_NAME}.dll ${CMAKE_SOURCE_DIR}/bin
)
add_dependencies(flame_physics copy_physx_dlls)

set_target_properties(flame_physics PROPERTIES FOLDER "flame") 
