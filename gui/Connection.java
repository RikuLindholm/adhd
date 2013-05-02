package gui;

import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import gui.FileMessage;
import java.nio.channels.SocketChannel;

public class Connection {

  public SocketChannel sock;
  public String host;
  public int port;

  public Connection() {
    this.host = "127.0.0.1";
    this.port = 52000;
  }

  public void connect() {
    try {
      this.sock = SocketChannel.open();
      this.sock.connect(new InetSocketAddress(this.host, this.port));
      this.sock.finishConnect();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public void disconnect() {
    System.out.println("Closing connection");
    try {
      this.sock.close();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public void sendMessage(ByteBuffer[] message) {
    try {
      System.out.println(message);
      this.sock.write(message);
    } catch (IOException err) {
      System.err.println(err);
    }
  }
}
