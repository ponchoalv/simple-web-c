# simple-web-c

Simple webserver written in C to start hacking around sockets and HTTP

The idea is to start with a simple webserver and then move forward to building a proxy.

Will start using normal sockets and threads. Then incrementally add more advance features and performance improvements, EPOLL, kqueue, etc.


HERE WE GO :rocket:

// TODO: Support more `mime-types`
// TODO: add tests :cringe:


## test

For tests run `make test`

It needs `cmocka` installed can be done by installing `sudo apt-get install libcmocka-dev` in Ubuntu. 

Other Os just google `cmocka install`

## Dependencies

For building the server you will need `libpthreadpool` which can be installed in ubuntu with `sudo apt-get install libpthreadpool-dev`.

//TODO: use a package manager as `conan` and `CMake` too lazy to learn about this two for now.
