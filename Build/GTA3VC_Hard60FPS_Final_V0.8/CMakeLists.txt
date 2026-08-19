cmake_minimum_required(VERSION 3.20)
project(GTA3_Hard60FPS LANGUAGES CXX)

add_library(GTA3_Hard60FPS SHARED GTA3_Hard60FPS.cpp)

target_compile_features(GTA3_Hard60FPS PRIVATE cxx_std_17)
target_compile_definitions(GTA3_Hard60FPS PRIVATE WIN32_LEAN_AND_MEAN)

set_target_properties(GTA3_Hard60FPS PROPERTIES
    OUTPUT_NAME "GTA3_Hard60FPS"
    PREFIX ""
)
