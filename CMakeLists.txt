cmake_minimum_required(VERSION 3.20)

project(test)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_PREFIX ${PROJECT_SOURCE_DIR})

#项目根目录
set(BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR})
set(SRC_DIR ${PROJECT_SOURCE_DIR})

#生成目录
set(EXECUTABLE_OUTPUT_PATH ${BIN_DIR})

get_filename_component(DIR_ONE_ABOVE ../ ABSOLUTE)

# include(${DIR_ONE_ABOVE}/7z/7z.cmake)
# include(${DIR_ONE_ABOVE}/util/util.cmake)
include_directories(${INCLUDE_DIR})

#exe配置
add_executable(test
    ${SRC_DIR}/bytestream.cpp
    ${SRC_DIR}/main.cpp
)