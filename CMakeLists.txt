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

if(MSVC)
    set(COMPILE_IGNORE_WARNING
        /wd4828
    )

    add_compile_options(
        $<$<CONFIG:>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
        $<$<C_COMPILER_ID:MSVC>:/utf-8>
        $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
        ${COMPILE_IGNORE_WARNING}
    )
endif()

include_directories(${INCLUDE_DIR})

#exe配置
add_executable(test
    ${SRC_DIR}/bytestream.cpp
    ${SRC_DIR}/main.cpp
)