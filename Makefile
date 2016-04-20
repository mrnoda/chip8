src = $(shell find src -name '*.c')
obj = $(src:.c=.o)

CFLAGS = -I./include -std=c99 -O3 -g -Werror -Wall -Wpedantic -Wno-unused-parameter
LDFLAGS = -lSDL2

bin/c8_emu: $(obj)
	mkdir -p bin
	cp resources/* bin/
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -rf $(obj) bin/
