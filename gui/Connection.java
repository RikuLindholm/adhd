import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SocketChannel;

public class Connection {

  private static SocketChannel sock;
  private static String host = "127.0.0.1";
  private static int port = 52000;

  public static void connect() {
    try {
      sock = SocketChannel.open();
      sock.connect(new InetSocketAddress(host, port));
      sock.finishConnect();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public static void disconnect() {
    try {
      sock.close();
    } catch (IOException err) {
      System.err.println(err);
    }
  }

  public static void write(ByteBuffer buffer) throws IOException {
    while (buffer.hasRemaining()) {
      sock.write(buffer);
    }
  }

  public static int read(ByteBuffer buffer) throws IOException {
    return sock.read(buffer);
  }
}
