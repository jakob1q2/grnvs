import java.io.IOException;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

public class test {


    public static void main(String[] args) {
        byte[] buf = new byte[20];
        buf[3] = (byte) 0x60;
        System.out.println(Integer.toBinaryString((buf[3] & 0xFF) + 0x100).substring(1));

        char[] a = new char[7];
        a[0] = 'a';
        char[] b = new char[]{'c', 'd'};
        System.arraycopy(b, 0, a, 1, 2);
        System.out.println(Arrays.toString(a));
        ByteBuffer bb = ByteBuffer.allocate(2);
        char i = 0x0;
        bb.putChar(i++);
        byte[] res = bb.array();
        System.out.println(Arrays.toString(res));
        bb.clear();
        bb.putChar(i++);
        res = bb.array();
        System.out.println(Arrays.toString(res));
        System.out.println((byte) 145);


    }
}