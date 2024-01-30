AR := tar
CXX := g++
CPPLINT := cpplint
CFLAGS := -Wall -Wextra -Werror -Wformat --std=c++17 -I include -pthread
CFLAGS_DEBUG := -g -DGUEST_DEBUG -DMONITOR_IOCTL -fsanitize=address


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
tested_src = $(filter-out src/main.cpp, $(src))
test_lib = $(subst lib,, $(basename $(notdir $(gtest_lib)))) pthread
CFLAGS_TEST = $(addprefix -l, $(test_lib)) -I $(gtest_include_dir) -L $(gtest_lib_dir) -DUNITTEST


include = include/baseclass.hpp \
		  include/boot.hpp \
		  include/cmos.hpp \
		  include/cpufeat.hpp \
		  include/iodev.hpp \
		  include/kvm.hpp \
		  include/paging.hpp \
		  include/pci.hpp \
		  include/pio.hpp \
		  include/post.hpp \
		  include/vcpu.hpp \
		  include/vm.hpp

src = src/main.cpp \
	  src/baseclass.cpp \
	  src/boot.cpp \
	  src/cmos.cpp \
	  src/iodev.cpp \
	  src/kvm.cpp \
	  src/pci.cpp \
	  src/pio.cpp \
	  src/post.cpp \
	  src/vm.cpp \
	  src/vcpu.cpp


$(gtest_dir):
	mkdir $@

$(gtest_tar_gz): $(gtest_dir)
	wget $(gtest_tar_gz_url) -O $@

$(gtest_release): $(gtest_tar_gz)
	# FIXME: Broken deps in gtest_lib
	$(AR) -C $(dir $<) -xf $<

$(gtest_lib): $(gtest_release)
	mkdir $(gtest_build_dir)
	cd $(gtest_build_dir) && cmake .. $(gtest_build_flags)
	$(MAKE) -C $(gtest_build_dir)

unittest: $(test_src) $(tested_src) $(gtest_lib)
	# ***NEVER CHANGE THE POSITION OF ARGUMENTS IN THE MAKEFILE AS WE ARE LINKING STATIC LIBS***
	$(CXX) $(CFLAGS) $(test_src) $(tested_src) $(CFLAGS_TEST) -o $@

unittest_debug: $(test_src) $(tested_src) $(gtest_lib)
	# NEVER CHANGE THE POSITION OF ARGUMENTS!!!
	$(CXX) $(CFLAGS) $(test_src) $(tested_src) $(CFLAGS_TEST) -o $@ -g

initramfs: scripts/geninitramfs.bash
	./scripts/geninitramfs.bash

lmigtester_debug: $(src) $(include)
	$(CXX) $(CFLAGS) $(CFLAGS_DEBUG) -o $@ $(src)

lmigtester: $(src) $(include)
	$(CXX) $(CFLAGS) $(src) -o $@

.PHONY: clean tag lint

clean:
	rm -f lmigtester lmigtester_debug initramfs unittest unittest_debug peda-session-* .gdb_history tags

tag:
	rm tags && ctags -R .

lint: $(src) $(include)
	$(CPPLINT) $^
