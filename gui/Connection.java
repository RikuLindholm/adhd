package gui;

import java.net.*;
import java.io.*;
import gui.FileMessage;

public class Connection {

  private Socket sock;
  private OutputStream out;
  public String host;
  public int port;

  public Connection() {
    this.host = "127.0.0.1";
    this.port = 52000;
  }

  public void connect() {
    try {
      this.sock = new Socket(this.host, this.port);
      this.out = sock.getOutputStream();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public void disconnect() {
    try {
      this.out.close();
      this.sock.close();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public void sendMessage(byte[] message) {
    try {
      System.out.println(message);
      this.out.write(message);
      this.out.flush();
    } catch (IOException err) {
      System.err.println(err);
    }
  }
}
