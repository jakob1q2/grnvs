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

    private static final byte versionIpv6 = (byte) 0x60; //version IPv6 (traffic class set 0)
    private static final byte icmpLength = (byte) 8; //length of ICMPv6 message in byte
    private static final byte icmpNH = (byte) 0x3a; //Next Header ICMPv6
    private static final byte icmpEchoRequ = (byte) 128; //ICMP type Echo Request
    private static final byte icmpEchoRep = (byte) 129; //ICMP type Echo Reply
    private static final byte icmpTimeEx = (byte) 3; //ICMP type Time Exceeded
    private static final byte icmpDestUnreach = (byte) 1; //ICMP type Destination unreachable
    private static final byte icmpID = (byte) 0xab; // ICMP Identifier in Byte 44-45, we modify only 44

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
            Timeout to = new Timeout(timeout);
            System.out.print(hops);
            for (int attempt = 1; attempt <= attempts; ++attempt) {

                putPerHopStaticIPContent(buffer, srcIp, dstIp, hops);

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

                boolean done = false;
                while (!done) {
                    ret = sock.read(buffer, to);

                    if (0 > ret) {
                        System.err.printf("failed to read from socket: %d\n", ret);
                        sock.hexdump(buffer, length);
                        System.exit(1);
                    }
                    if (to.getTimeout() == 0) {
                        System.out.print("  *");
                        done = true;
                    } else {
                        done = checkMessage(buffer, done);
                    }
                }
            }

        }
        /*===========================================================================*/

    }

    private static void putPerHopStaticIPContent(byte[] buffer, byte[] srcIp, byte[] dstIp, int hops) {
        buffer[0] = versionIpv6;
        buffer[4] = icmpLength;
        buffer[6] = icmpNH;
        buffer[7] = (byte) hops;
        System.arraycopy(srcIp, 0, buffer, 8, 16);
        System.arraycopy(dstIp, 0, buffer, 24, 16);

        buffer[40] = icmpEchoRequ;
        buffer[41] = (byte) 0x00; // ICMP Code
        buffer[44] = icmpID;
    }

    private static boolean checkMessage(byte[] buffer, boolean done) {
        int pos = 6;
        int skip = 34;
        while (buffer[pos] == (byte) 0x00 || buffer[pos] == (byte) 0x2b || buffer[pos] == (byte) 0x3c) {
            pos += skip;
            skip = 8 + 8 * (int) buffer[pos + 1];
        }
        if (buffer[pos] == icmpNH) {
            pos += skip;
            switch (buffer[pos]) {
                case icmpTimeEx:
                    handleIcmpTimeEx(buffer, pos, done);
                    break;
                case icmpDestUnreach:
                    handleIcmpDestUnreach(buffer, pos, done);
                    break;
                case icmpEchoRep:
                    handleIcmpEchoRep(buffer, pos, done);
                    break;
                default:
            }
        }
        return done;
    }

    private static boolean handleIcmpTimeEx(byte[] buffer, int pos, boolean done) {
        System.out.println();
        return done;
    }

    private static boolean handleIcmpDestUnreach(byte[] buffer, int pos, boolean done) {
        return done;
    }

    private static boolean handleIcmpEchoRep(byte[] buffer, int pos, boolean done) {
        return done;
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
