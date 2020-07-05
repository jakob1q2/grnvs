import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;

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
    private static final char icmpLength = 8; //length of ICMPv6 message in byte
    private static final byte icmpNH = (byte) 0x3a; //Next Header ICMPv6
    private static final byte icmpEchoRequ = (byte) 128; //ICMP type Echo Request
    private static final byte icmpEchoRep = (byte) 129; //ICMP type Echo Reply
    private static final byte icmpTimeEx = (byte) 3; //ICMP type Time Exceeded
    private static final byte icmpDestUnreach = (byte) 1; //ICMP type Destination unreachable
    private static final byte icmpID = (byte) 0xab; // ICMP Identifier in Byte 44-45, we modify only 44
    private static boolean stopProbes;

    public static void run(GRNVS_RAW sock, String dst, int timeout,
                           int attempts, int hopLimit) {
        ByteBuffer bb16 = ByteBuffer.allocate(16);
        bb16.order(ByteOrder.BIG_ENDIAN);
        bb16.put(sock.getIPv6(), 0, 16);
        byte[] buffer = new byte[1514];
        int length = 48; //was set 0?
        byte[] dstIp = new byte[16];
        byte[] srcIp = bb16.array();
        bb16.clear();
        byte[] ipHeader = new byte[40];
        byte[] payload = new byte[8];

        System.out.println(dst); //////////////////////////////////

        /*====================================TODO===================================*/

        /* TODO:
         * 1) Initialize the addresses required to build the packet
         * 2) Loop over hoplimit and attempts
         * 3) Build and send the packet for each iteration
         * 4) Print the hops found in the specified format
         */
        try {
            bb16.put(InetAddress.getByName(dst).getAddress(), 0, 16);
            dstIp = bb16.array();
            bb16.clear();
            System.out.println(Arrays.toString(dstIp));/////////////
            System.out.println(Arrays.toString(InetAddress.getByName(dst).getAddress()));///////
        } catch (UnknownHostException e) {
            e.printStackTrace();
        }

        ByteBuffer bb2 = ByteBuffer.allocate(2);
        bb2.order(ByteOrder.BIG_ENDIAN);
        stopProbes = false;
        ipHeader[0] = versionIpv6;

        bb2.putChar(icmpLength);
        byte[] l = bb2.array();
        bb2.clear();
        System.arraycopy(l, 0, ipHeader, 4, 2);

        ipHeader[6] = icmpNH;
        System.arraycopy(srcIp, 0, ipHeader, 8, 16);
        System.arraycopy(dstIp, 0, ipHeader, 24, 16);

        payload[0] = icmpEchoRequ;
        payload[1] = (byte) 0x00; // ICMP Code
        payload[4] = icmpID;

        int hops = 1;
        char sequenceNumber = 0x0;

        while (!stopProbes && hops <= hopLimit) {
            ipHeader[7] = (byte) hops;
            Timeout to = new Timeout(timeout);
            System.out.print(hops);
            for (int attempt = 1; attempt <= attempts; ++attempt) {

                bb2.putChar(sequenceNumber++);
                byte[] sequNum = bb2.array();
                bb2.clear();
                System.arraycopy(sequNum, 0, payload, 6, 2);

                System.arraycopy(ipHeader, 0, buffer, 0, 40);
                System.arraycopy(payload, 0, buffer, 40, 8);

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
                        try {
                            done = checkMessage(buffer, done);
                        } catch (UnknownHostException e) {
                            e.printStackTrace();
                        }
                    }
                }
            }
            System.out.print("\n");
            hops++;
        }
        /*===========================================================================*/

    }


    private static boolean checkMessage(byte[] buffer, boolean done) throws UnknownHostException {
        int pos = 6;
        int skip = 34;
        while (buffer[pos] == (byte) 0x00 || buffer[pos] == (byte) 0x2b || buffer[pos] == (byte) 0x3c) {
            pos += skip;
            skip = 8 + 8 * (int) buffer[pos + 1];
        }
        if (buffer[pos] == icmpNH) {
            pos += skip;
            byte[] src = new byte[16];
            System.arraycopy(buffer, 8, src, 0, 16);
            String host = InetAddress.getByAddress(src).getHostAddress();
            switch (buffer[pos]) {
                case icmpTimeEx:
                    done = handleIcmpTimeEx(buffer, pos, host);
                    break;
                case icmpDestUnreach:
                    done = handleIcmpDestUnreach(buffer, pos, host);
                    break;
                case icmpEchoRep:
                    done = handleIcmpEchoRep(buffer, pos, host);
                    break;
                default:
            }
        }
        return done;
    }

    private static boolean handleIcmpTimeEx(byte[] buffer, int pos, String host) {
        System.out.print("  " + host);
        return true;
    }

    private static boolean handleIcmpDestUnreach(byte[] buffer, int pos, String host) {
        stopProbes = true;
        System.out.print("  " + host + "!X");
        return true;
    }

    private static boolean handleIcmpEchoRep(byte[] buffer, int pos, String host) {
        System.out.print("  " + host);
        stopProbes = true;
        return true;
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
