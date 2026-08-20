cmake_minimum_required(VERSION 3.20)
project(SA_HardFPS_v1_1 LANGUAGES CXX)

add_library(SA_HardFPS_v1_1 SHARED
    SA_HardFPS_v1_1.cpp
)

target_compile_features(SA_HardFPS_v1_1 PRIVATE cxx_std_17)
target_compile_definitions(SA_HardFPS_v1_1 PRIVATE WIN32_LEAN_AND_MEAN)
target_link_libraries(SA_HardFPS_v1_1 PRIVATE d3d9)

set_target_properties(SA_HardFPS_v1_1 PROPERTIES
    OUTPUT_NAME "SA_HardFPS_v1_1"
)
