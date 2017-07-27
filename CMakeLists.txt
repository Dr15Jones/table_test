cmake_minimum_required (VERSION 2.6)
project (SOATable)

#only fast-math seems to be needed to help vectorization
#SET(GCC_VECTORIZE "-ftree-vectorize -ffast-math -march=native")
SET(GCC_VECTORIZE "-ffast-math")

SET(ROOTSYS "$ENV{ROOTSYS}")
SET(ROOT_INCLUDES "-I ${ROOTSYS}/include")
SET(ROOT_LIBS "-L ${ROOTSYS}/lib -l RIO -l Tree -l Core")

set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ROOT_INCLUDES} ${GCC_VECTORIZE}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ROOT_LIBS}" )
include_directories ("${PROJECT_SOURCE_DIR}")

add_executable(branch_test branch_test.cc)
add_executable(branch_read_test branch_read_test.cc)
add_executable(table_test table_test.cc)
add_executable(table_reader_test table_reader_test.cc)
add_executable(table_time_test table_time_test.cc)

#set(CMAKE_BUILD_TYPE Release)
set(CMAKE_BUILD_TYPE Debug)
