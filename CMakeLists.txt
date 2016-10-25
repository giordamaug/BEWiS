cmake_minimum_required(VERSION 2.8)
# change paths to your C++ compiler
# set( CMAKE_C_COMPILER /opt/local/bin/gcc-mp-5 )
# set( CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-5 )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 " )

project( bewis )

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find OpenMP
find_package(OpenMP)

if(OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Build wizard Library
# Build bemis program
set(BGWIZSRC_NAME wisardlib/bgwis.cpp)
set(MAINSRC_NAME bewis.cpp)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./wisardlib)

set(LIB_NAME wisard)
set(BIN_NAME bewis)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
add_library(${LIB_NAME} SHARED ${BGWIZSRC_NAME} wisardlib/wisard.cpp wisardlib/wnet_lib.cpp)
target_link_libraries( ${LIB_NAME} ${OpenCV_LIBS})

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
add_executable(${BIN_NAME} ${MAINSRC_NAME})
target_link_libraries(${BIN_NAME} ${LIB_NAME} ${OpenCV_LIBS})
