cmake_minimum_required(VERSION 3.5)
project(BrownCode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE_FILES
        readers/AbstractReader.h
        readers/ReaderFrequency.cpp
        readers/ReaderFrequency.h
        readers/ReaderNoOrder.cpp
        readers/ReaderNoOrder.h
        tree/InnerNode.cpp
        tree/InnerNode.h
        tree/LeafNode.cpp
        tree/LeafNode.h
        tree/Node.cpp
        tree/Node.h
        tree/RootNode.cpp
        tree/RootNode.h
        BrownClusteringAlgorithm/BrownClusteringAlgorithm.cpp
        BrownClusteringAlgorithm/BrownClusteringAlgorithm.h
        models/Corpus.cpp
        models/Corpus.h
        models/WordMappings.cpp
        models/WordMappings.h
        Utils.cpp
        Utils.h
        ExchangeAlgorithm/ExchangeAlgorithm.h
        ExchangeAlgorithm/ExchangeAlgorithm.cpp
        ExchangeAlgorithm/Exchange/Exchange.cpp
        ExchangeAlgorithm/Exchange/Exchange.h
        ExchangeAlgorithm/StochasticExchange/StochasticExchange.cpp
        ExchangeAlgorithm/StochasticExchange/StochasticExchange.h
        CorpusUtils.cpp
        CorpusUtils.h
        readers/ReaderNoOrderSkip.h
        readers/ReaderNoOrderSkip.cpp
        readers/ReaderThreshold.h
        readers/ReaderThreshold.cpp
        readers/ReaderClusteringIntoCorpus.h
        readers/ReaderClusteringIntoCorpus.cpp
        )

include_directories(../libs/)

add_library(BrownCode STATIC ${SOURCE_FILES})

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

IF (CMAKE_BUILD_TYPE STREQUAL "Coverage")
    MESSAGE("Because build type is ${CMAKE_BUILD_TYPE}, including code coverage flags")
    # Add flags for coverage
    SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")# -Wall -pedantic")
    SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
ENDIF() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"