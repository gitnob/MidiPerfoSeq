cmake_minimum_required(VERSION 3.0)

project(perfoseq)

add_executable(perfoseq main.cpp)

install(TARGETS perfoseq RUNTIME DESTINATION bin)
