cmake_minimum_required(VERSION 3.25)
project(RISC-Z)

set(SRC_LIST main.c risc-z.c memory.c ecall.c)

add_executable(risc-z ${SRC_LIST})