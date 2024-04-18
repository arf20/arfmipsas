cmake_minimum_required(VERSION 3.10)

project(arfmipsas)

file(GLOB SRC "*.c")

add_executable(arfmipsas ${SRC})
