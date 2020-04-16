cmake_minimum_required(VERSION 3.14)
project(os2)

set(CMAKE_CXX_STANDARD 14)

add_executable(os2 main.cpp uthreads.cpp uthreads.h thread.cpp thread.h)