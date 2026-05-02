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

install: z_jail
	mkdir -p $(DESTDIR)/usr/local/bin $(DESTDIR)/usr/local/share/man/man1
	cp z_jail $(DESTDIR)/usr/local/bin/
	cp man/z_jail.1 $(DESTDIR)/usr/local/share/man/man1/

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/z_jail
	rm -f $(DESTDIR)/usr/local/share/man/man1/z_jail.1

dist: z_jail
	mkdir -p _dist
	cp z_jail README.md LICENSE _dist/
	cp -r man completions _dist/
	tar czf _dist/z-jail-$(ZJAIL_VERSION).tar.gz -C _dist z_jail README.md LICENSE man completions

pre-commit-check:
	@echo "Checking for tab indentation..."
	@grep -rn "        " include/ src/ --include="*.c" --include="*.h" | grep -v "Binary" || true
	@echo "OK"

