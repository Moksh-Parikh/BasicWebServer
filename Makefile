CC=gcc
CFLAGS=
DEPS=server_functions.h
OBJ = server.o server_functions.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build/main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY : clean
clean :
	-rm $(OBJ)
