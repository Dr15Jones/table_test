cmake_minimum_required (VERSION 2.6)
project (SOATable)

#only fast-math seems to be needed to help vectorization
#SET(GCC_VECTORIZE "-ftree-vectorize -ffast-math -march=native")
SET(GCC_VECTORIZE "-ffast-math")

#https://root.cern.ch/how/integrate-root-my-project-cmake
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT REQUIRED COMPONENTS RIO Tree Core)
include(${ROOT_USE_FILE})

#Generating the dictionary requires a recent CMSSW_9_3_ROOT6 version
REFLEX_GENERATE_DICTIONARY(G__JetTable classes.h SELECTION classes_def.xml)
REFLEX_GENERATE_DICTIONARY(G__Wrapper classes_wrapper.h SELECTION classes_def_wrapper.xml)


#SET(ROOTSYS "$ENV{ROOTSYS}")
#SET(ROOT_INCLUDES "-I ${ROOTSYS}/include")
#SET(ROOT_LIBS "-L ${ROOTSYS}/lib -l RIO -l Tree -l Core")

set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED ON)

#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${ROOT_INCLUDES} ${GCC_VECTORIZE}" )
SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_VECTORIZE}" )
SET( CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ROOT_LIBS}" )
include_directories ("${PROJECT_SOURCE_DIR}")

add_executable(branch_test branch_test.cc)
target_link_libraries(branch_test ${ROOT_LIBRARIES})

add_executable(branch_read_test branch_read_test.cc)
target_link_libraries(branch_read_test ${ROOT_LIBRARIES})

add_library(G__JetTableDict SHARED G__JetTable.cxx)
target_link_libraries(G__JetTableDict ${ROOT_LIBRARIES})

add_library(G__WrapperDict SHARED G__Wrapper.cxx)
target_link_libraries(G__WrapperDict ${ROOT_LIBRARIES})

add_executable(table_test table_test.cc)
add_executable(table_reader_test table_reader_test.cc)
add_executable(table_time_test table_time_test.cc)
add_executable(branch_struct_write_test branch_struct_write_test.cc)
target_link_libraries(branch_struct_write_test ${ROOT_LIBRARIES})

add_executable(branch_struct_read_test branch_struct_read_test.cc)
target_link_libraries(branch_struct_read_test ${ROOT_LIBRARIES})

add_executable(branch_wrapper_write_test branch_wrapper_write_test.cc)
target_link_libraries(branch_wrapper_write_test ${ROOT_LIBRARIES})

add_executable(branch_wrapper_read_test branch_wrapper_read_test.cc)
target_link_libraries(branch_wrapper_read_test ${ROOT_LIBRARIES})

set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_BUILD_TYPE Debug)
