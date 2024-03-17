CC=gcc
CFLAGS=-Wall -I. -lpthreadpool
DEPS=
OBJ=server.o
TEST_DBG_OBJ=test_dbg.o
TEST_SERVER_OBJ=test_server.o
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

test_dbg: $(TEST_DBG_OBJ)
		$(CC) -o $@ $^ $(CFLAGS) -lcmocka
		./$@

test_server: $(TEST_SERVER_OBJ)
		$(CC) -o $@ $^ server.o $(CFLAGS) -lcmocka
		./$@
