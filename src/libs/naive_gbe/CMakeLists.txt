#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.1)
project(naive_gbe)

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
