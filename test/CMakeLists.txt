#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.1)
project(test_gbe)

cmake_policy(SET CMP0057 NEW)

aux_source_directory(
	.
	SRC_LIST)

find_package(
	Threads REQUIRED)

include(GoogleTest)

include_directories(
	../src/libs/naive_gbe/include
	${GTEST_INCLUDE_DIR})

SET(COVERAGE OFF CACHE BOOL "Coverage")

add_executable(
	${PROJECT_NAME}
	${SRC_LIST})

gtest_add_tests(${PROJECT_NAME} "" AUTO)

if (COVERAGE)
    target_compile_options(
		${PROJECT_NAME} PRIVATE --coverage)

    target_link_libraries(
		${PROJECT_NAME} PRIVATE --coverage
		gtest
		${CMAKE_THREAD_LIBS_INIT})
else()
	target_link_libraries(
		${PROJECT_NAME}
		gtest
		${CMAKE_THREAD_LIBS_INIT})
endif()
