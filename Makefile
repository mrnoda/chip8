src = $(shell find src -name '*.c')
obj = $(src:.c=.o)

CFLAGS = -I./include --std=c99 -g -Wpedantic
LDFLAGS = -lSDL2

bin/c8_emu: $(obj)
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(obj) bin/
