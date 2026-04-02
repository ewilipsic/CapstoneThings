cmake_minimum_required(VERSION 3.16)
project(pybind_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g1")


# Find Python + pybind11 (pip installed)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
execute_process(COMMAND "${Python3_EXECUTABLE}" -m pybind11 --cmakedir
                OUTPUT_VARIABLE PYBIND11_DIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_DIR}")
find_package(pybind11 REQUIRED)

# Build Python extension module
# pybind11_add_module(example example.cpp)
pybind11_add_module(tsn binding.cpp make_inputs.cpp algo.cpp yens.cpp new_algo.cpp)
# 1. Force Release build to drop the massive debug symbols (Highly Recommended)
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)

# 2. Add "big object" support based on your compiler
if (MSVC)
    target_compile_options(tsn PRIVATE /bigobj)
elseif (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # For MinGW / GCC / Clang
    target_compile_options(tsn PRIVATE -Wa,-mbig-obj)
    
    # Force strip debug symbols specifically for the target if Release type isn't enough
    target_compile_options(tsn PRIVATE -g0) 
endif()
