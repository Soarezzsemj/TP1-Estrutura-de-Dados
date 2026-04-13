cmake_minimum_required(VERSION 3.10)
project(TP1_Estrutura_de_Dados C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(include .)

add_executable(TP1_Estrutura_de_Dados
        include/Libs.h
        src/Main.c
        src/Prop.c)


