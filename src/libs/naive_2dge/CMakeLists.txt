#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.1)
project(naive_2dge)

find_package(SDL2 REQUIRED)

find_package(PkgConfig)
pkg_check_modules(SDL2_IMG REQUIRED SDL2_image)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)
pkg_check_modules(SDL2_MIXER REQUIRED SDL2_mixer)

aux_source_directory(
	.
	SRC_LIST)

include_directories(
	include)

add_library(
	${PROJECT_NAME}
	${SRC_LIST})

install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX})
install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION lib)
