#
#            Copyright (c) Marco Amorim 2020.
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
project(naive_gbe)
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

install(DIRECTORY src/naive_gbe DESTINATION ${CMAKE_INSTALL_PREFIX})

include(gtest.cmake)
enable_testing()

add_subdirectory(test)
add_subdirectory(src)

find_package(Doxygen)

if(DOXYGEN_FOUND)

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in
        ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)

endif(DOXYGEN_FOUND)

