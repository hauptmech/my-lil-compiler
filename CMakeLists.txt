cmake_minimum_required(VERSION 2.6)
PROJECT(mlc) #My-Lil-Compiler
set(CMAKE_INSTALL_PREFIX "~" CACHE PATH "Local install prefix" FORCE)
add_subdirectory(src)
add_subdirectory(lib)

