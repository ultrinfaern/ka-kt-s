cmake_minimum_required(VERSION 3.27)
project(ka_kt_s)

set(CMAKE_CXX_STANDARD 23)

add_executable(ka_kt_s main.cpp
        ar.h
        ar.cpp)

set(CMAKE_EXE_LINKER_FLAGS " -static")
target_link_libraries(ka_kt_s -static-libgcc -static-libstdc++)
