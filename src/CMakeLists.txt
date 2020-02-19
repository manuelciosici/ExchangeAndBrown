cmake_minimum_required(VERSION 3.5)
project(Brown)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")
message("CMAKE_CXX_STANDARD is ${CMAKE_CXX_STANDARD}")

# Remove default myeasylog.log file
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DELPP_NO_DEFAULT_LOG_FILE")

# Optimise to compiling machine
SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -mavx")

# Add flags for coverage
#SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")

set(SOURCE_FILES
        libs/easylogging++/easylogging++.cc)

add_subdirectory(${CMAKE_SOURCE_DIR}/src)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/libs/)

##########################################
#  executables and targets for experiments
##### Brown main
add_executable(Brown main.cpp ${SOURCE_FILES})
target_link_libraries(Brown BrownCode)
##### Simple Brown
add_executable(simple_brown simple_brown.cpp ${SOURCE_FILES})
target_link_libraries(simple_brown BrownCode)
##### Exchange runner
add_executable(exchange_runner exchange_runner.cpp ${SOURCE_FILES})
target_link_libraries(exchange_runner BrownCode)
##### Brown over clusters
add_executable(compute_brown_over_clusters experiment_runners/compute_brown_over_clusters.cpp ${SOURCE_FILES})
target_link_libraries(compute_brown_over_clusters BrownCode)
##### Clustering facts
add_executable(clustering_facts experiment_runners/clustering_facts.cpp ${SOURCE_FILES})
target_link_libraries(clustering_facts BrownCode)
##### Print clustering AMI
add_executable(print_clustering_ami experiment_runners/print_clustering_ami.cpp ${SOURCE_FILES})
target_link_libraries(print_clustering_ami BrownCode)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Commented out until I fix the UNI desktop
# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    set (DOCUMENTATION_PATH "${CMAKE_CURRENT_BINARY_DIR}")
    message(status "** Will write documentation to ${DOCUMENTATION_PATH}")
    add_custom_target(documentation ALL
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${DOCUMENTATION_PATH}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
endif(DOXYGEN_FOUND)
