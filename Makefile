AR := tar
CXX := g++
CFLAGS := -Wall -Wextra -Werror --std=c++17 -I include
CFLAGS_DEBUG := -g -DMONITOR_IOCTL


gtest_dir := gtest
gtest_tar_gz_url := https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
gtest_build_flags := -DBUILD_GMOCK=OFF
gtest_tar_gz = $(gtest_dir)/v1.14.0.tar.gz
gtest_release = $(gtest_dir)/googletest-1.14.0
gtest_build_dir = $(gtest_release)/build
gtest_lib_files = libgtest.a libgtest_main.a
gtest_lib_dir = $(gtest_build_dir)/lib
gtest_lib = $(addprefix $(gtest_lib_dir)/, $(gtest_lib_files))
gtest_include_dir = $(gtest_release)/googletest/include

test_dir := test
test_src = $(wildcard $(test_dir)/*.cpp)
tested_src = $(addprefix src/, $(filter-out gtest_main.cpp, $(notdir $(test_src))))
test_lib = $(subst lib,, $(basename $(notdir $(gtest_lib)))) pthread
CFLAGS_TEST = $(addprefix -l, $(test_lib)) -I $(gtest_include_dir) -L $(gtest_lib_dir)


include = include/baseclass.hpp \
		  include/bios.hpp \
		  include/cpufeat.hpp \
		  include/kvm.hpp \
		  include/vcpu.hpp \
		  include/vm.hpp

src = src/main.cpp \
	  src/baseclass.cpp \
	  src/bios.cpp \
	  src/kvm.cpp \
	  src/vm.cpp \
	  src/vcpu.cpp


$(gtest_dir):
	mkdir $@

$(gtest_tar_gz): $(gtest_dir)
	wget $(gtest_tar_gz_url) -O $@

$(gtest_release): $(gtest_tar_gz)
	$(AR) -C $(dir $<) -xf $<

$(gtest_lib): $(gtest_release)
	mkdir $(gtest_build_dir)
	cd $(gtest_build_dir) && cmake .. $(gtest_build_flags)
	$(MAKE) -C $(gtest_build_dir)

unittest: $(test_src) $(tested_src) $(gtest_lib)
	# NEVER CHANGE THE POSITION OF ARGUMENTS!!!
	$(CXX) $(CFLAGS) $(test_src) $(tested_src) $(CFLAGS_TEST) -o $@

initramfs: scripts/geninitramfs.bash
	./scripts/geninitramfs.bash

supermigrator_debug: $(src) $(include)
	$(CXX) $(CFLAGS) $(CFLAGS_DEBUG) -o $@ $(src)

supermigrator: $(src) $(include)
	$(CXX) $(CFLAGS) $(src) -o $@


.PHONY: clean

clean:
	rm -f supermigrator supermigrator_debug initramfs unittest
