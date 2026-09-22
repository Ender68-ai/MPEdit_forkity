cmake_minimum_required(VERSION 3.21)
project(advanced_label)

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} INTERFACE)

target_sources(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/AdvancedLabel.cpp)
target_include_directories(${PROJECT_NAME} INTERFACE include)