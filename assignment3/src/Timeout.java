 public class Timeout {
	/* This entire is only used as mutable int wrapper */
	private int timeout;

	public Timeout(int milliseconds) {
		this.timeout = milliseconds;
	}

	public int getTimeout() { return timeout; }
	public void setTimeout(int timeout) { this.timeout = timeout; }
}
