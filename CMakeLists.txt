cmake_minimum_required(VERSION 4.2)
project(Electronics_Store_Management_System)

set(CMAKE_CXX_STANDARD 20)


add_executable(Electronics_Store_Management_System
        main.cpp
        Inventory.cpp
        Product.cpp
        productFactory.cpp
        productFactory.h
        Product.h)
