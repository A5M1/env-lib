CC=gcc
CFLAGS=-Wall -O2 -I./env
SRC=$(wildcard env/*.c)
OBJ=$(SRC:.c=.o)
LIB=libenv.a

TEST_SRC=$(wildcard test/*.c)
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST_TARGET=testProgram

all: $(LIB) $(TEST_TARGET) docs

$(LIB): $(OBJ)
	ar rcs $@ $^

$(TEST_TARGET): $(TEST_OBJ) $(LIB)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

docs:
	doxygen Doxyfile

clean:
	rm -f $(OBJ) $(TEST_OBJ) $(LIB) $(TEST_TARGET)
	rm -rf docs/html/*
