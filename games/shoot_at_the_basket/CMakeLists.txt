project(shoot_at_the_basket)

file(GLOB_RECURSE SHOOT_AT_THE_BASKET_HEADER_LIST "src/*.h*")
file(GLOB_RECURSE SHOOT_AT_THE_BASKET_SOURCE_LIST "src/*.c*")

group_source("${SHOOT_AT_THE_BASKET_HEADER_LIST}" "/src" "Header")
group_source("${SHOOT_AT_THE_BASKET_SOURCE_LIST}" "/src" "Source")

add_executable(shoot_at_the_basket ${SHOOT_AT_THE_BASKET_HEADER_LIST} ${SHOOT_AT_THE_BASKET_SOURCE_LIST})

target_link_libraries(shoot_at_the_basket flame_surface)
target_link_libraries(shoot_at_the_basket flame_graphics)
target_link_libraries(shoot_at_the_basket flame_model)
target_link_libraries(shoot_at_the_basket flame_physics)

set_target_properties(shoot_at_the_basket PROPERTIES FOLDER "games") 
set_target_properties(shoot_at_the_basket PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")