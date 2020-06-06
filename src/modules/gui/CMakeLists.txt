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
	${SDL2_LIBRARIES}
    ${SDL2_IMG_LIBRARIES}
    ${SDL2_TTF_LIBRARIES}
    ${SDL2_MIXER_LIBRARIES})

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME naive_gbe)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
