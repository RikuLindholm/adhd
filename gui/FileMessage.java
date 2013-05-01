package gui;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.DataInputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import gui.Connection;

public class FileMessage {

  private File file;
  private String key;
  private int length;
  private byte[] data;
  private static final int SAVE = 1;
  private static final int FETCH = 2;

  public FileMessage(File file) {
    this.file = file;
    this.key = "A94A8FE5CCB19BA61C4C0873D391E987982FBBD3"; // sha1(file.getName())
  }

  // Read file into memory
  private void readFile() {
    this.length = (int) file.length();
    try {
      this.data = new byte[(int) this.length];
      DataInputStream stream = new DataInputStream(new FileInputStream(this.file));
      stream.readFully(this.data);
      stream.close();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  // Write data into a file
  private void writeFile() {
    return;
  }

  // Fetch single block from DHT
  public void fetch() {
    return;
  }

  // Save block to DHT
  public void save() {
    readFile();
    Connection conn = new Connection();
    conn.connect();
    conn.sendMessage(ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(SAVE).array()); // Send type
    conn.sendMessage(ByteBuffer.allocate(8).order(ByteOrder.LITTLE_ENDIAN).putLong(this.length).array()); // Send length
    //conn.sendMessage(this.data); // Send data
    conn.disconnect();
    return;
  }
}
