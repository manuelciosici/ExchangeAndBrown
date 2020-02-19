project(brown_tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake_modules")

IF (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    MESSAGE("Because build type is ${CMAKE_BUILD_TYPE}, including CodeCoverage")
    INCLUDE(CodeCoverage)

    # Add flags for coverage
    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage -Wall -pedantic")
    SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")

    SETUP_TARGET_FOR_COVERAGE(
            ${PROJECT_NAME}_coverage  # Name for custom target.
            testRunner         # Name of the test driver executable that runs the tests.
            # NOTE! This should always have a ZERO as exit code
            # otherwise the coverage generation will not complete.
            coverage            # Name of output directory.
    )
ENDIF() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"


add_subdirectory(lib/googletest)
#--------------------
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR} ${gmock_SOURCE_DIR}/include ${gmock_SOURCE_DIR})
include_directories(../src)
include_directories(../libs/)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message("CMAKE_CXX_STANDARD is ${CMAKE_CXX_STANDARD}")

set(SOURCE_FILES
        tests/main.cpp
        ../../libs/easylogging++/easylogging++.cc
        tests/TestUtil.cpp
        tests/TestUtil.h
        tests/TestBrownClusteringAlgorithm.cpp
        tests/TestBrownClusteringAlgorithm.h
        tests/TestExchange.cpp
        tests/TestCorpus.cpp
        tests/TestCorpusUtils.cpp
        tests/TestTree.cpp
        tests/TestWordMappings.cpp
        tests/TestReaderThreshold.cpp
        tests/TestReaderNoOrderSkip.cpp
        )
#
set(CMAKE_VERBOSE_MAKEFILE ON)

# Test runner
add_executable(testRunner ${SOURCE_FILES})
target_link_libraries(testRunner BrownCode gtest gtest_main)

add_custom_command(TARGET testRunner POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data
        $<TARGET_FILE_DIR:testRunner>/test_data
        )

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    MESSAGE("Including OpenMP by using ${OpenMP_CXX_FLAGS}")
endif()
#--------------------