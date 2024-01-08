/*
 *  src/baseclass.cpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#include <baseclass.hpp>

#include <unistd.h>

#include <iostream>


BaseClass::BaseClass(int fd) : fd(fd) {}

BaseClass::~BaseClass() {
    if (fd >= 0) {
        close(fd);
        std::cout << "closed: " << fd << std::endl;
    }
}
