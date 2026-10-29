# perspecta_see — build with any C99 compiler.
#
#   make            build ./perspecta_see
#   make run        build and serve on http://127.0.0.1:8787
#   make demo       build and serve on 0.0.0.0 (sandbox / container previews)
#   make clean

CC      ?= cc
CFLAGS  ?= -std=c99 -O2 -Wall -Wextra -Iinclude
LDFLAGS ?=

SRC  := $(wildcard src/*.c)
OBJ  := $(SRC:.c=.o)
BIN  := perspecta_see

ifeq ($(OS),Windows_NT)
  BIN := perspecta_see.exe
  LDFLAGS += -lws2_32
endif

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: $(BIN)
	./$(BIN) --port 8787

demo: $(BIN)
	./$(BIN) --port 8787 --host 0.0.0.0

clean:
	rm -f $(OBJ) $(BIN) perspecta_see.exe

.PHONY: all run demo clean
