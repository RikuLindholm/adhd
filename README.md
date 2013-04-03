A Distributed Hash Device (ADHD)
====

ADHD is a distributed file store client implementation.

## Running the server

A java implementation of the DHT server is given as a JAR package.
You can start the server by running:

    java -jar DHTServer.jar <port>

## Running the client

### Dependencies

For generating the SHA-1 hashes and compiling the client you'll need to install
the Libgcrypt cryptography library. On OSX the easiest way is through Homebrew:

    brew install libgcrypt

Google for instructions on Linux/Windows ;)

### Compiling and running the client

The client can be compiled easily by running:

    make -f Makefile

This compiles the code into an executable named 'main' which can be run with:

  ./main <server-address> <server-port> <my-address> <my-port>

Note that the server needs to be running for proper functionality.
