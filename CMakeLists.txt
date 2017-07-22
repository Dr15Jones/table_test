cmake_minimum_required (VERSION 2.6)
project (SOATable)
SET(CXX_STL_14 "-std=c++14")
#SET(ROOTSYS "/cvmfs/cms.cern.ch/slc6_amd64_gcc530/lcg/root/6.08.07/")
SET(ROOTSYS "$ENV{ROOTSYS}")
SET(ROOT_INCLUDES "-I ${ROOTSYS}/include")
SET(ROOT_LIBS "-L ${ROOTSYS}/lib -l RIO -l Tree -l Core")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${CXX_STL_14} ${ROOT_INCLUDES}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ROOT_LIBS}" )
include_directories ("${PROJECT_SOURCE_DIR}")

#add_executable(branch_test branch_test.cc)
add_executable(table_test table_test.cc)