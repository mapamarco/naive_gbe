#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
project(core)
cmake_minimum_required(VERSION 3.1)

aux_source_directory(
	.
	SRC_LIST)

include_directories(
	../libs/naive_gbe/include
	${GTEST_INCLUDE_DIR})

add_executable(
	${PROJECT_NAME}
	${SRC_LIST})

target_link_libraries(
	${PROJECT_NAME}
	naive_gbe)