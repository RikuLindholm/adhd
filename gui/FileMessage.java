import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.DataInputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CharsetDecoder;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.channels.FileChannel;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.logging.Logger;
import java.util.logging.Level;

public class FileMessage {

    private File file;
    private String key;
    private byte[] data;
    private static final int FILE_GET = 21;
    private static final int FILE_PUT = 22;
    private static final int FILE_NOT_FOUND = 28;
    private static final Logger logger = Logger.getLogger("gui");

    public FileMessage(File file) {
        this.file = file;
        try {
            this.key = sha1(file.getName());
        } catch (NoSuchAlgorithmException err) {
            logger.log(Level.SEVERE, "Could not create SHA1 hash for file");
            return;
        }
    }

    private String sha1(String value) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA-1");
        return byteArrayToHexString(md.digest(value.getBytes()));
    }

    private String byteArrayToHexString(byte[] b) {
        String result = "";
        for (int i=0; i < b.length; i++) {
          result += Integer.toString( ( b[i] & 0xff ) + 0x100, 16).substring( 1 );
        }
        return result;
    }

    /**
    * Read local file into memory
    */
    private void readFile() throws IOException {
        this.data = new byte[(int) this.file.length()];
        DataInputStream stream = new DataInputStream(new FileInputStream(this.file));
        stream.readFully(this.data);
        stream.close();
    }

    /**
    * Write data block into a local file
    */
    private void writeFile() throws IOException {
        FileChannel out = new FileOutputStream("response.txt").getChannel();
        ByteBuffer buff2 = ByteBuffer.allocate(1024);
        int retval = 1;
        while (retval > 0) {
            retval = Connection.read(buff2);
            buff2.flip();
            out.write(buff2);
            buff2.clear();
        }
        logger.log(Level.INFO, "Successfully saved response.txt");
        out.close();
    }

    /**
    * Fetch single data block from DHT
    */
    public void fetch() throws IOException {
        ByteBuffer buffer = ByteBuffer.allocate(4 + 40).order(ByteOrder.LITTLE_ENDIAN);
        Charset set = Charset.forName("UTF-8");
        CharsetEncoder encoder = set.newEncoder();
        CharsetDecoder decoder = set.newDecoder();

        buffer.putInt(FILE_GET);
        buffer.put(encoder.encode(CharBuffer.wrap(this.key)));
        buffer.flip();

        Connection.write(buffer);

        ByteBuffer buff = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN);
        int read = 0;
        while (read < 4) {
          read += Connection.read(buff);
        }

        buff.flip();
        if (buff.getInt() == FILE_NOT_FOUND) {
            throw new FileNotFoundException("File could not be found in the DHT");
        } else {
            writeFile();
        }
    }

    /**
    * Store a data block into DHT
    */
    public void save() throws IOException {
        readFile();
        ByteBuffer buffer = ByteBuffer.allocate(4 + 40 + 4 + (int)this.file.length()).order(ByteOrder.LITTLE_ENDIAN);
        Charset set = Charset.forName("UTF-8");
        CharsetEncoder encoder = set.newEncoder();

        buffer.putInt(FILE_PUT);
        buffer.put(encoder.encode(CharBuffer.wrap(this.key)));
        buffer.putInt((int)this.file.length());
        buffer.put(this.data);
        buffer.flip();

        Connection.write(buffer);
    }
}
