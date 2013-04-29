package gui;

import java.net.*;
import java.io.*;

import gui.MessageTypes;

public class Connection {

  Socket sock;
  OutputStream out;
  String host;
  int port;

  public Connection() {
    this.host = "127.0.0.1";
    this.port = 52000;
  }

  public void connect() {
    try {
      sock = new Socket(this.host, this.port);
      out = sock.getOutputStream();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public void disconnect() {
    try {
      out.close();
      sock.close();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public void sendMessage(int type, String data) {
    connect();
    try {
      char[] message = Integer.toString(type).toCharArray();
      out.write(message[0]);
      out.flush();
    } catch (IOException err) {
      System.err.println(err);
    }
    disconnect();
  }

}
