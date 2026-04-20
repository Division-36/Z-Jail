CC := gcc
CFLAGS := -O2 -g -I include
LDFLAGS := -Wl,-z,now -Wl,-z,relro -pie

SRC := src/z_jail.c
OBJ := $(SRC:.c=.o)
TARGET := z_jail

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
