#include <iostream>
#include <unistd.h>

#include <baseclass.hpp>


BaseClass::BaseClass(int fd) : fd(fd) {}

BaseClass::~BaseClass() {
    if (fd >= 0) {
        close(fd);
        std::cout << "closed fd: " << fd << std::endl;
    }
}
