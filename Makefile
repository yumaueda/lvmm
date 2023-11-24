AR := tar
CXX := g++
CFLAGS := -Wall -Wextra -Werror --std=c++17 -I include
CFLAGS_DEBUG := -g -DMONITOR_IOCTL


googletest_dir := gtest
googletest_tar_gz_url := https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
googletest_build_flags := -DBUILD_GMOCK=OFF

googletest_tar_gz = $(googletest_dir)/v1.14.0.tar.gz
googletest_release = $(googletest_dir)/googletest-1.14.0
googletest_build_dir = $(googletest_release)/build


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


$(googletest_dir):
	mkdir $@

$(googletest_tar_gz): $(googletest_dir)
	wget $(googletest_tar_gz_url) -O $@

$(googletest_release): $(googletest_tar_gz)
	$(AR) -C $(dir $<) -xf $<

googletest_lib: $(googletest_release)
	mkdir $(googletest_build_dir)
	cd $(googletest_build_dir) && cmake .. $(googletest_build_flags)
	$(MAKE) -C $(googletest_build_dir)


initramfs: scripts/geninitramfs.bash
	./scripts/geninitramfs.bash

supermigrator: $(src) $(include)
	$(CXX) $(CFLAGS) -o $@ $(src)

supermigrator_debug: $(src) $(include)
	$(CXX) $(CFLAGS) $(CFLAGS_DEBUG) -o $@ $(src)


clean:
	rm -f supermigrator supermigrator_debug initramfs


.PHONY: clean
