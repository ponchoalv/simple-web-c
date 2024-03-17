CC=gcc
CFLAGS=-I.
DEPS=
OBJ=server.o
TEST_OBJ=test.o
USERID=123456789

%.o: %.c $(DEPS)
				$(CC) -c -o $@ $< $(CFLAGS)

all: server
server: $(OBJ)
				$(CC) -o $@ $^ $(CFLAGS)

clean:
				rm -rf *.o server *.tar.gz

dist: tarball
tarball: clean
				tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .

debug: $(OBJ)
		$(CC) -o $@ $^ $(CFLAGS) -g

test: $(TEST_OBJ)
		$(CC) -o $@ $^ $(CFLAGS) -lcmocka
		./test