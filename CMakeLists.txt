cmake_minimum_required(VERSION 3.25)
project(os_hw1)

set(CMAKE_CXX_STANDARD 11)

add_executable(os_hw1 smash.cpp Commands.cpp Commands.h signals.cpp signals.h)
