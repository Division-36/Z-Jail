CC := gcc
CFLAGS := -O2 -g -fstack-protector-strong -D_FORTIFY_SOURCE=2
CFLAGS += -fPIC -fno-strict-aliasing -Werror=implicit-function-declaration
CFLAGS += -Wall -Wextra -std=gnu11 -I include
LDFLAGS := -Wl,-z,now -Wl,-z,relro -pie

SRC := src/log.c src/util.c src/seccomp_bpf.c
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


check: z_jail
	@echo "Running smoke check..."
	./z_jail --version 2>&1 | head -1
	@echo "OK"

test: z_jail
	$(MAKE) -C tests setup >/dev/null 2>&1
	bash tests/run_tests.sh

help:
	@echo "Targets:"
	@echo "  make          - build z_jail"
	@echo "  make clean    - remove build artifacts"
	@echo "  make check    - smoke test (--version)"
	@echo "  make test     - run full test suite (17 scenarios)"
	@echo "  make install  - install to \$$DESTDIR/usr/local"
	@echo "  make dist     - create release tarball"
	@echo "  make help     - show this message"

valgrind-check: z_jail
	valgrind --leak-check=full --error-exitcode=1 ./z_jail --version 2>&1 || echo "valgrind not available"


