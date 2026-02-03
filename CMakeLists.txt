cmake_minimum_required(VERSION 4.1)
project(CS50x C)

set(CMAKE_C_STANDARD 17)

# Target 1: The original main program
add_executable(main_prog main.c)

# Target 2: The hello program (needs cs50.c to work)
add_executable(hello_prog hello.c cs50.c cs50.h)


add_executable(agree_prog agree.c cs50.c cs50.h)
add_executable(launcher_prog launcher.c cs50.c cs50.h)
add_executable(types_prog playground/types.c cs50.c cs50.h)
