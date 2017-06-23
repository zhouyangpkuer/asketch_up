cmake_minimum_required (VERSION 2.6)
project(NMExp)

include(check_AVX.cmake)

FindSSE()

if(CMAKE_COMPILER_IS_GNUCXX)
    message(STATUS "GCC detected, adding compile flags")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -Wmissing-declarations -Wredundant-decls")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mssse3 -msse4.1 -msse4.2 -mavx")
        if (AVX2)
            MESSAGE(STATUS "AVX2 set")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -mavx2 -march=core-avx2")
        endif()
#    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -O3")
	ADD_DEFINITIONS(-DUNIX)
endif(CMAKE_COMPILER_IS_GNUCXX)

ADD_DEFINITIONS(-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
SET(SOURCE_FILES src/solutions.h)
set(SOURCE_FILES ${SOURCE_FILES})

add_executable(main ${SOURCE_FILES} src/main.cpp)
