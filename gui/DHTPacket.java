package gui;

public class DHTPacket {

  public String target; // 20 bytes SHA-1
  public String sender; // Not used in UI-node interaction
  public int type;
  public int length;    // The length of the data string
  public String data;
  
  // Constructor that constructs a packet object
  // from a serialized string representation
  public DHTPacket(String serialized) {
    this.target = serialized.substring(0,20);
    this.sender = serialized.substring(20,40);
    this.type = (((int)serialized.charAt(40) & 0xff) << 8)
                  | ((int)serialized.charAt(41) & 0xff);
    this.length = (((int)serialized.charAt(42) & 0xff) << 8)
                  | ((int)serialized.charAt(43) & 0xff);
    this.data = serialized.substring(44, 44 + this.length);
  }
  
  // Static utility function that constructs a serialized packet string.
  // String target must be of length 20 or null.
  // String data can be of any length under 2^16 - 1 or null.
  public static String serialize(String target, int type, String data) {
    int length = data.length();
    StringBuffer sb = new StringBuffer();
    if (target == null || target.length() != 20)
      sb.append(new char[20]); // Append zero key
    else
      sb.append(target);
    sb.append(new char[20]); // Append the unused key
    sb.append((char)((type >> 8) & 0xff));
    sb.append((char)(type & 0xff));
    sb.append((char)((length >> 8) & 0xff));
    sb.append((char)(length & 0xff));
    sb.append(data);
    return sb.toString();
  }

}
