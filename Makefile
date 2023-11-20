CC = g++
CFLAGS = -Wall -Wextra -Werror --std=c++17 -I include

initramfs: scripts/geninitramfs.bash
	./scripts/geninitramfs.bash

SRC = src/main.cpp \
	  src/baseclass.cpp \
	  src/kvm.cpp \
	  src/vm.cpp \
	  src/vcpu.cpp

supermigrator: $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f supermigrator initramfs

.PHONY: clean
