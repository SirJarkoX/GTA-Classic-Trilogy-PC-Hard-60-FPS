cmake_minimum_required(VERSION 3.20)
project(GTA3VC_HardFPS_v1_1 LANGUAGES CXX)

add_library(GTA3VC_HardFPS_v1_1 SHARED
    GTA3VC_HardFPS_v1_1.cpp
)

target_compile_features(GTA3VC_HardFPS_v1_1 PRIVATE cxx_std_17)
target_compile_definitions(GTA3VC_HardFPS_v1_1 PRIVATE WIN32_LEAN_AND_MEAN)
