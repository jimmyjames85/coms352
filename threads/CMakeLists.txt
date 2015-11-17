cmake_minimum_required(VERSION 3.2)
project(threads)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    badcnt.c
    list.c
    list.h
    llist.c
    llist.h
    main.c)

add_executable(threads ${SOURCE_FILES} old_main.c)