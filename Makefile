CC = g++
CFLAGS = -Wall -Wextra -Werror --std=c++17 -I include

INCLUDE = include/baseclass.hpp \
		  include/cpufeat.hpp \
		  include/kvm.hpp \
		  include/vcpu.hpp \
		  include/vm.hpp

SRC = src/main.cpp \
	  src/baseclass.cpp \
	  src/kvm.cpp \
	  src/vm.cpp \
	  src/vcpu.cpp

supermigrator: $(SRC) $(INCLUDE)
	$(CC) $(CFLAGS) -o $@ $(SRC)

supermigrator_debug: $(SRC) $(INCLUDE)
	$(CC) $(CFLAGS) -o $@ $(SRC) -DMONITOR_IOCTL

initramfs: scripts/geninitramfs.bash
	./scripts/geninitramfs.bash

clean:
	rm -f supermigrator supermigrator_debug initramfs

.PHONY: clean
