CC=gcc
DEPS= src/ctcp/server.h
OBJ= src/ctcp/server.c src/ctcp/main.c
CFLAGS = -g \
		-W \
		-Wall \
		-Wextra \
		-Wpedantic \
		-Wshadow \
		-Wwrite-strings \
		-Wstrict-prototypes \
		-Wmissing-prototypes \
		-Wmissing-declarations \
		-Wno-unused-parameter \
		-Wshadow \
		-Wold-style-definition \
		-Wredundant-decls \
		-Wnested-externs \
		-Wmissing-include-dirs \
		-D_GNU_SOURCE \
		-D_REENTRANT \
		-D_LINUX_ \
		-O2

%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

ctcp: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
