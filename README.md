A Distributed Hash Device (ADHD)
====

ADHD is a distributed file store client implementation.

## Quick Instructions

For quickly testing the client and GUI, please run:

  make                # Compile client and GUI
  make run            # Run client
  make run-gui        # Run GUI

## Running the server

A test server is constantly running at dht.mikkokivela.com:80 so there is no actual need to run the server locally.

If you would however like to run the server locally, you can start it by running:

    java -jar DHTServer.jar <port>

## Compiling the client and GUI

The client and the accompanying GUI can be compiled easily by running:

    make

This compiles the code into an executable named 'dhtonode'.

## Running the client

The C client can be started easily by running:

    make run  // Defaults to options: dht.mikkokivela.com 80 127.0.0.1 50000

Alternatively, to manually define the client options (e.g. for connecting to a local server), it can be run with:
 
    ./dhtonode <server-address> <server-port> <my-address> <my-port>

Note that the server needs to be running for proper functionality.

## Running the GUI

For running the accompanying GUI you can simply run:

    make run-gui

Please make sure that both the server and the C client are running before starting the GUI.
