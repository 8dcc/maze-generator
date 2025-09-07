
CC     := gcc
CFLAGS := -std=c99 -Wall -Wextra -Wpedantic -Wshadow
LDLIBS := -lpng

SRC := main.c vec.c maze_ctx.c image.c
OBJ := $(addprefix obj/, $(addsuffix .o, $(SRC)))

BIN=maze-generator.out

#-------------------------------------------------------------------------------

.PHONY: clean all

all: $(BIN)

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

#-------------------------------------------------------------------------------

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<
