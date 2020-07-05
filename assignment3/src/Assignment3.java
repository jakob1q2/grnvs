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
    private static byte[] myIP;

    public static void run(GRNVS_RAW sock, String dst, int timeout,
                           int attempts, int hopLimit) throws UnknownHostException {
        byte[] buffer;
        int length = 48;
        byte[] dstIp = InetAddress.getByName(dst).getAddress();
        byte[] srcIp = sock.getIPv6();
        myIP = srcIp;
        byte[] ipHeader = new byte[40];
        byte[] payload = new byte[8];


        /*====================================TODO===================================*/

        /* TODO:
         * 1) Initialize the addresses required to build the packet
         * 2) Loop over hoplimit and attempts
         * 3) Build and send the packet for each iteration
         * 4) Print the hops found in the specified format
         */

        ByteBuffer bb2 = ByteBuffer.allocate(2);
        bb2.order(ByteOrder.BIG_ENDIAN);
        stopProbes = false;
        int hops = 1;
        char sequenceNumber = 0x0;
        Timeout to = new Timeout(timeout * 1000);

        while (!stopProbes && hops <= hopLimit) {
            System.out.print(hops);
            for (int attempt = 1; attempt <= attempts; attempt++) {
                buffer = new byte[1514];

                //build header
                ipHeader[0] = versionIpv6;

                bb2.putChar(icmpLength);
                byte[] l = bb2.array();
                bb2.clear();
                System.arraycopy(l, 0, ipHeader, 4, 2);

                ipHeader[6] = icmpNH;
                ipHeader[7] = (byte) hops;
                System.arraycopy(srcIp, 0, ipHeader, 8, 16);
                System.arraycopy(dstIp, 0, ipHeader, 24, 16);

                //build payload
                payload[0] = icmpEchoRequ;
                payload[1] = (byte) 0x00; // ICMP Code
                payload[4] = icmpID;

                to.setTimeout(timeout * 1000);
                bb2.putChar(sequenceNumber++);
                byte[] sequNum = bb2.array();
                bb2.clear();
                System.arraycopy(sequNum, 0, payload, 6, 2);

                //build packet
                System.arraycopy(ipHeader, 0, buffer, 0, 40);
                System.arraycopy(payload, 0, buffer, 40, 8);

                byte[] cksum = GRNVS_RAW.checksum(buffer, 0, buffer, 40, length - 40);

                System.arraycopy(cksum, 0, buffer, 42, 2);

                //send
                int ret = sock.write(buffer, length);

                if (0 >= ret) {
                    System.err.printf("failed to write to socket: %d\n", ret);
                    sock.hexdump(buffer, length);
                    System.exit(1);
                }
                //start read
                boolean done = false; //done if timeout or valid response
                while (!done) {
                    ret = sock.read(buffer, to);

                    if (0 > ret) {
                        System.err.printf("failed to read from socket: %d\n", ret);
                        sock.hexdump(buffer, length);
                        System.exit(1);
                    }
                    if (ret == 0) {
                        System.out.print("  *"); //timeout
                        done = true;
                    } else {
                        byte[] resp = new byte[ret];
                        System.arraycopy(buffer, 0, resp, 0, ret);
                        done = checkMessage(resp);
                    }
                }
            }
            System.out.print("\n");
            hops++;
        }
        /*===========================================================================*/

    }


    private static boolean checkMessage(byte[] buffer) throws UnknownHostException {
        //check if ipv6
        if (buffer[0] != (byte) 0x60) {
            return false;
        }
        //check dest address
        /**
         byte[] rec = new byte[16];
         System.arraycopy(buffer, 24, rec, 0, 16);
         String receiver = InetAddress.getByAddress(rec).getHostAddress();
         System.out.println("receiver " + receiver + " and my: " + InetAddress.getByAddress(myIP).getHostAddress()); ////////////////////////////////////

         if (!InetAddress.getByAddress(myIP).getHostAddress().equals(receiver)) { //message not for me
         return false;
         }
         */

        boolean done = false;

        //search for Icmp message
        int pos = 6;
        int skip = 34;
        while (buffer[pos] == (byte) 0x00 || buffer[pos] == (byte) 0x2b || buffer[pos] == (byte) 0x3c) { //skip valid extension headers
            pos += skip;
            skip = 8 + 8 * (int) buffer[pos + 1]; //extension header length in byte
        }
        if (buffer[pos] == icmpNH) {
            pos += skip;
            //boolean relevant = checkProperties(buffer, pos);
            boolean relevant = true;
            if (relevant) {
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
                    default: //ignore
                        System.out.println("ignored bc default"); ////////////////////////////////////
                }
            }
        }
        return done;
    }

    private static boolean checkProperties(byte[] buffer, int pos) {
        //check checksum
        byte[] sum = new byte[2];
        System.arraycopy(buffer, pos + 2, sum, 0, 2);
        buffer[pos + 2] = 0;
        buffer[pos + 3] = 0;
        byte[] cksum = GRNVS_RAW.checksum(buffer, 0, buffer, pos, buffer.length - 40);
        if (cksum[0] != sum[0] || cksum[1] != sum[1]) {
            return false;
        }
        return true;
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
