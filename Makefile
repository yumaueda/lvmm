CC = g++
CFLAGS = -Wall -Wextra -Werror --std=c++17 -I include

SRC = src/main.cpp \
	  src/baseclass.cpp \
	  src/kvm.cpp \
	  src/vm.cpp \
	  src/vcpu.cpp

supermigrator: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

supermigrator_debug: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ -DMONITOR_IOCTL

initramfs: scripts/geninitramfs.bash
	./scripts/geninitramfs.bash

clean:
	rm -f supermigrator supermigrator_debug initramfs

.PHONY: clean
