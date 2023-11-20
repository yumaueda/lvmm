NAME = supermigrator
CC = g++
CFLAGS = -Wall -Wextra -Werror --std=c++17 -I include

SRC = src/main.cpp \
	  src/baseclass.cpp \
	  src/kvm.cpp \
	  src/vm.cpp \
	  src/vcpu.cpp

$(NAME): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(NAME)

.PHONY: clean
