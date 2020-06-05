#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
set(GOOGLETEST_ROOT gtest/googletest CACHE STRING "Google Test source root")

include_directories(SYSTEM
	${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}
	${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}/include)

set(GOOGLETEST_SOURCES
	${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}/src/gtest-all.cc
	${PROJECT_SOURCE_DIR}/${GOOGLETEST_ROOT}/src/gtest_main.cc)

foreach(_source ${GOOGLETEST_SOURCES})
	set_source_files_properties(${_source} PROPERTIES GENERATED 1)
endforeach()

add_library(gtest ${GOOGLETEST_SOURCES})
