public class GRNVS_RAW {
	static {
		System.loadLibrary("GRNVS");
	}

	private final int handle;

	private final native int getSocket(String dev, int level);
	private final native int write_(int handle, byte[] buffer, int size);
	private final native int read_(int handle, byte[] buffer, Timeout time);
	private final native int close_(int handle);
	private final native byte[] mac_(int handle);
	private final native byte[] ip_(int handle);
	private final native byte[] ip6_(int handle);

	private static final native void hexdump_(byte[] buffer, int size);
	private static final native byte[] checksum_(byte[] hdr, int hdrOffset,
			byte[] payload, int payloadOffset, int payloadLength);
	private static final int SOCK_RAW = 3;
	private static final int SOCK_DGRAM = 2;

	public GRNVS_RAW(String dev, int level) {
		handle = getSocket(dev, level);
	}

	/* Write to the network device
	 * @param buffer A bytebuffer containing the frame/packet
	 * @size the first size byte will be sent
	 *
	 * @result The number of bytes sent
	 */
	public final int write(byte[] buffer, int size) {
		return write_(handle, buffer, size);
	}

	/* Read from the network device
	 * @param buffer The buffer to read into
	 * @param Timeout The timeout that should be used for the read
	 *        it will be updated
	 *
	 * @result The number of bytes sent
	 */
	public final int read(byte[] buffer, Timeout time) {
		return read_(handle, buffer, time);
	}

	/* Get the ip address of the device for this socket */
	public final byte[] getIP() {
		return ip_(handle);
	}

	/* Get an ipv6 address of the device for this socket */
	public final byte[] getIPv6() {
		return ip6_(handle);
	}

	/* Get the mac address of the device for this socket */
	public final byte[] getMac() {
		return mac_(handle);
	}

	public final int close() {
		return close_(handle);
	}

	/* Dump a hex representation of the buffers content to stderr */
	public static final void hexdump(byte[] buffer, int size) {
		GRNVS_RAW.hexdump_(buffer, size);
	}

	/* Compute the icmpv6 checksum for the packet.
	 * @param hdr The buffer containing the ipv6 header
	 * @param hdrOffset the offset of the header in the buffer
	 * @param payload The buffer containing the icmp data (ip payload)
	 * @param payloadOffset the offset of the payload in the buffer
	 * @param payloadLength The length of the ip payload
	 *
	 * For more information refer to rfc1071.
	 * Important note: It is assumed that the field for the checksum is set to 0.
	 *
	 * @return A byte array containing the checksum, in the same order it
	 *         should be in the output buffer
	 */
	public static final byte[] checksum(byte[] hdr, int hdrOffset,
			byte[] payload, int payloadOffset, int payloadLength) {
		if (hdr == null)
			throw new NullPointerException("hdr must not be null.");
		if (payload == null)
			throw new NullPointerException("payload must not be null.");
		if (hdrOffset < 0)
			throw new IllegalArgumentException("hdrOffset must not be negative.");
		if (payloadOffset < 0)
			throw new IllegalArgumentException("payloadOffset must not be negative.");
		if (payloadLength < 0)
			throw new IllegalArgumentException("payloadLength must not be negative.");
		if (payloadLength > 2048)
			throw new IllegalArgumentException("payloadLength is too big.");
		if (hdr.length - hdrOffset < 40)
			throw new IllegalArgumentException("hdr must contain at least 40 bytes.");
		if (payload.length - payloadOffset < payloadLength)
			throw new IllegalArgumentException("payload is shorter than payloadLength.");

		return GRNVS_RAW.checksum_(hdr, hdrOffset, payload,
						payloadOffset, payloadLength);
	}

	protected void finalize() {
		close_(handle);
	}
}
