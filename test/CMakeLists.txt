#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
project(test_gbe)
cmake_minimum_required(VERSION 3.1)
cmake_policy(SET CMP0057 NEW)

aux_source_directory(
	.
	SRC_LIST)

find_package(
	Threads REQUIRED)

include(GoogleTest)

include_directories(
	../src/include
	${GTEST_INCLUDE_DIR})

add_executable(
	${PROJECT_NAME}
	${SRC_LIST})

target_link_libraries(
	${PROJECT_NAME}
	gtest
	gtest_main
	${CMAKE_THREAD_LIBS_INIT})

gtest_add_tests(${PROJECT_NAME} "" AUTO)

