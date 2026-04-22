CC := gcc
CFLAGS := -O2 -g -fstack-protector-strong -D_FORTIFY_SOURCE=2
CFLAGS += -fPIC -fno-strict-aliasing -Werror=implicit-function-declaration
CFLAGS += -Wall -Wextra -Wpedantic -std=c11 -I include
LDFLAGS := -Wl,-z,now -Wl,-z,relro -pie

SRC := src/log.c src/util.c src/truthimatics.c src/seccomp_bpf.c
SRC += src/audit.c src/sandbox.c src/z_jail.c
OBJ := $(SRC:.c=.o)
TARGET := z_jail

.PHONY: all clean test build_id
all: $(TARGET)
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
%.o: %.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@
-include $(SRC:.c=.d)
clean:
	rm -f $(OBJ) $(SRC:.c=.d) $(TARGET)
build_id:
	@printf "Build ID: Z-Jail/v1+dev\n"
