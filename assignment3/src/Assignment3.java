import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

public class Assignment3 {

    /**
     * This is the entry point for student code.
     * We do highly recommend splitting it up into multiple functions.
     * <p>
     * A good rule of thumb is to make loop bodies functions and group operations
     * that work on the same layer into functions.
     * <p>
     * For reading from the network have a look at assignment2. Also read the
     * comments in GRNVS_RAW.java
     * <p>
     * To get your own IP address use the getIPv6 function.
     * This one is also documented in GRNVS_RAW.java
     */
    public static void run(GRNVS_RAW sock, String dst, int timeout,
                           int attempts, int hopLimit) {
        byte[] buffer = new byte[1514];
        int length = 48; //was set 0?
        byte[] dstIp;
        byte[] srcIp;
        byte[] ipHeader;
        byte[] payload;


        /*====================================TODO===================================*/

        /* TODO:
         * 1) Initialize the addresses required to build the packet
         * 2) Loop over hoplimit and attempts
         * 3) Build and send the packet for each iteration
         * 4) Print the hops found in the specified format
         */
        srcIp = sock.getIPv6();
        try {
            dstIp = InetAddress.getByName(dst).getAddress();
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }

        int hops = 0;
        char sequenceNumber = 0x0;
        ByteBuffer bb = ByteBuffer.allocate(2);

        boolean dstReached = false;

        while (!dstReached && hops <= hopLimit) {
            for (int attempt = 1; attempt <= attempts; ++attempt) {

                buffer[0] = (byte) 0x60; //version IPv6 (traffic class set 0)
                buffer[4] = (byte) 8; //length of ICMPv6 message in byte
                buffer[6] = (byte) 0x3a; //Next Header ICMPv6
                buffer[7] = (byte) hops;
                System.arraycopy(srcIp, 0, buffer, 8, 16);
                System.arraycopy(dst, 0, buffer, 24, 16);

                buffer[40] = (byte) 128; //ICMP type Echo Request
                buffer[41] = (byte) 0x00; // ICMP Code
                buffer[44] = (byte) 0xab; // ICMP Identifier in Byte 44-45


                bb.putChar(sequenceNumber++);
                byte[] sequNum = bb.array();
                bb.clear();
                System.arraycopy(sequNum, 0, buffer, 46, 2);

                byte[] cksum = GRNVS_RAW.checksum(buffer, 0, buffer, 40, length - 40);

                System.arraycopy(cksum, 0, buffer, 42, 2);


                int ret = sock.write(buffer, length);

                if (0 >= ret) {
                    System.err.printf("failed to write to socket: %d\n", ret);
                    sock.hexdump(buffer, length);
                    System.exit(1);
                }
            }

        }
        /*===========================================================================*/

    }


    public static void main(String[] argv) {
        Arguments args = new Arguments(argv);
        GRNVS_RAW sock = null;
        try {
            sock = new GRNVS_RAW(args.iface, 2);
            run(sock, args.dst, args.timeout, args.attempts,
                    args.hoplimit);
            sock.close();
        } catch (Exception e) {
            e.printStackTrace();
            System.err.println(e.getMessage());
            System.exit(1);
        }
    }
}
