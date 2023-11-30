#include <iostream>
#include <unistd.h>
#include <baseclass.hpp>


BaseClass::BaseClass(int fd) : fd(fd) {}
BaseClass::~BaseClass() {}
