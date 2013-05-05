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
import java.lang.UnsupportedOperationException;
import java.util.logging.Logger;
import java.util.logging.Level;

public class FileMessage {

    private File file;
    private String key;
    private static final int HEADER_LEN = 44;
    private static final int FILE_GET = 21;
    private static final int FILE_PUT = 22;
    private static final int FILE_NOT_FOUND = 28;
    private static final int MAX_BLOCK_SIZE = 65535;
    private static final Logger logger = Logger.getLogger("gui");
    private static final CharsetEncoder encoder = Charset.forName("UTF-8").newEncoder();
    private static final CharsetDecoder decoder = Charset.forName("UTF-8").newDecoder();

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
    private void writeFileToChannel(int fileLength) throws IOException {
        FileChannel in = new FileInputStream(this.file).getChannel();
        ByteBuffer buffer = ByteBuffer.allocate(1024);
        int written = 0;

        while (written < fileLength) {
            written += in.read(buffer);
            buffer.flip();
            Connection.write(buffer);
            buffer.clear();
        }

        in.close();
    }

    /**
    * Write data block into a local file
    */
    private void readFileFromChannel(int fileLength, File targetFile) throws IOException {
        FileChannel out = new FileOutputStream(targetFile).getChannel();
        ByteBuffer buffer = ByteBuffer.allocate(1024);
        int read = 0;

        while (read < fileLength) {
            read += Connection.read(buffer);
            buffer.flip();
            out.write(buffer);
            buffer.clear();
        }

        out.close();
    }

    /**
    * Fetch single data block from DHT
    */
    public void fetch(File targetFile) throws IOException {
        ByteBuffer header = ByteBuffer.allocate(HEADER_LEN).order(ByteOrder.LITTLE_ENDIAN);

        header.putInt(FILE_GET);
        header.put(encoder.encode(CharBuffer.wrap(this.key)));
        header.flip();
        Connection.write(header);

        // Read response code
        ByteBuffer buffer = ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN);
        Connection.read(buffer);
        buffer.flip();

        if (buffer.getInt() == FILE_NOT_FOUND) {
            throw new FileNotFoundException("File could not be found in the DHT");
        } else {
            // Read file length
            buffer.clear();
            Connection.read(buffer);
            buffer.flip();
            readFileFromChannel(buffer.getInt(), targetFile);
        }
    }

    /**
    * Store a data block into DHT
    */
    public void save() throws IOException {
        ByteBuffer header = ByteBuffer.allocate(HEADER_LEN + 4).order(ByteOrder.LITTLE_ENDIAN);
        int fileLength = (int)this.file.length();
        if (fileLength > MAX_BLOCK_SIZE)
            throw new IOException("File too large. Multi-block transfer not supported.");
        header.putInt(FILE_PUT);
        header.put(encoder.encode(CharBuffer.wrap(this.key)));
        header.putInt(fileLength);
        header.flip();
        Connection.write(header);

        writeFileToChannel(fileLength);
    }
}
