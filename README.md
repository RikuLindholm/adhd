A Distributed Hash Device (ADHD)
====

ADHD is a distributed file store client implementation.

## Running the server

A java implementation of the DHT server is given as a JAR package.
You can start the server by running:

    java -jar DHTServer.jar <port>

## Compiling and running the client

The client can be compiled easily by running:

    make

This compiles the code into an executable named 'dhtonode' which can be run with:

  make run  // Defaults to options: 127.0.0.1 3200 127.0.0.1 50000

Alternatively, to manually define the client options, it can be run with:
 
  ./dhtonode <server-address> <server-port> <my-address> <my-port>

Note that the server needs to be running for proper functionality.
