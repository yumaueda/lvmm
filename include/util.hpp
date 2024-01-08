/*
 *  include/util.hpp
 *
 *  Copyright (C) 2023  Yuma Ueda <cyan@0x00a1e9.dev>
 */


#ifndef INCLUDE_UTIL_HPP_
#define INCLUDE_UTIL_HPP_


#include <fstream>
#include <ios>
#include <iostream>


static inline std::streamsize get_ifs_size(std::ifstream& file) {
    std::ios::pos_type original_pos = file.tellg();

    if (!file) {
        std::cerr << __func__ << ": invalid ifstream given" << std::endl;
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();

    file.seekg(original_pos, std::ios::beg);

    return size;
}

static inline bool is_kernel_elf(std::ifstream& kernel) {
    char buf[ELF_MAGIC_SIZE];
    std::ios::pos_type original_pos = kernel.tellg();

    if (!kernel) {
        std::cerr << __func__ << ": invalid ifstream given" << std::endl;
        return false;
    }

    kernel.seekg(0, std::ios::beg);
    kernel.read(buf, ELF_MAGIC_SIZE);
    kernel.seekg(original_pos);

    return std::equal(buf, buf+ELF_MAGIC_SIZE, ELF_MAGIC);
};


#endif  // INCLUDE_UTIL_HPP_
