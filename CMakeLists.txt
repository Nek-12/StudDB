cmake_minimum_required(VERSION 3.15)
project(Stud)

set(CMAKE_CXX_STANDARD 20)

set(GCC_COVERAGE_COMPILE_FLAGS "-O3 -static -Wall -Wextra -Wfloat-equal -Wundef -Wcast-align -Wwrite-strings -Wlogical-op -Wredundant-decls -Wshadow -Woverloaded-virtual")
#
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")

add_executable(Stud Class.h utility.cpp main.cpp lib/sha256.h lib/sha256.cpp Data.cpp Entry.cpp lib/fort.hpp lib/fort.h lib/fort.c misc.h)
