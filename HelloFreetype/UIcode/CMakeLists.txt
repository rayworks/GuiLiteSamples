cmake_minimum_required(VERSION 2.8)

PROJECT(UIcode)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

FILE(GLOB SOURCE *.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/freetype_include)

ADD_LIBRARY(UIcode  ${SOURCE})

execute_process(
    COMMAND ${CMAKE_SOURCE_DIR}/.sync_build.sh HelloFreetype
)
