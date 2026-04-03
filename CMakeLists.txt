cmake_minimum_required(VERSION 4.0)
project(TP1_Estrutura_de_Dados C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(TP1_Estrutura_de_Dados
        Libs.h
        Main.c
        Prop.c)
