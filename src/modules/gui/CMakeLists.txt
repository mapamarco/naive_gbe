#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.1)
project(naive_gbe_bin)

aux_source_directory(
	.
	SRC_LIST)

include_directories(
	../../libs/naive_2dge/include
	../../libs/naive_gbe/include)

find_package(SDL2 REQUIRED)

find_package(PkgConfig)
	
pkg_check_modules(SDL2_IMG REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

add_executable(
	${PROJECT_NAME}
	${SRC_LIST})

target_link_libraries(
	${PROJECT_NAME}
	naive_gbe
	naive_2dge
	${SDL2_LIBRARIES}
    ${SDL2_IMG_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES})

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME naive_gbe)

add_custom_command(
	TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/assets/* ${CMAKE_CURRENT_BINARY_DIR}
)

install(
	DIRECTORY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
	TARGETS ${PROJECT_NAME} DESTINATION bin)
