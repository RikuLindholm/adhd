import java.net.*;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.channels.SocketChannel;

public class Connection {

  private static SocketChannel sock;

  public static void connect(int port) throws IOException {
    sock = SocketChannel.open();
    sock.connect(new InetSocketAddress("127.0.0.1", port));
    sock.finishConnect();
  }

  public static void disconnect() throws IOException {
    sock.close();
  }

  public static void write(ByteBuffer buffer) throws IOException {
    while (buffer.hasRemaining()) {
      sock.write(buffer);
    }
  }

  public static int read(ByteBuffer buffer) throws IOException {
    return sock.read(buffer);
  }
 
    public static void read(ByteBuffer buffer, int length) throws IOException {
        int read = 0;
        while (read < length) {
            read += sock.read(buffer);
        }
    }
}
