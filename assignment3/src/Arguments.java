import java.lang.NumberFormatException;

class Arguments {
	public String dst;
	public String iface = "eth0";
	public int timeout = 5;
	public int hoplimit = 15;
	public int attempts = 3;

	private void printHelp() {
		System.out.println("Traceroute for GRNVS");
		System.out.println("Usage:");
		System.out.println("-i/--interface: The interface on which the frame should be sent");
		System.out.println("-t             The timeout that should be used for timed actions (default: 5)");
		System.out.println("-q             The attempts that should be done per hop (default: 3)");
		System.out.println("-m             The maximum hops to trace over (default: 15)");
		System.out.println("DST:            The destination IPv6 address ");
		System.out.println("-?/--help	    Print this help message");
	}

	Arguments(String[] argv) {
		if(argv.length == 0) {
			printHelp();
			System.exit(0);
		}
		//For_each would be nice, but we may have to skip/access next
		int i, j = 0;
		String[] fargs = new String[1];
		for(i = 0; i < argv.length; ++i) {
			String arg = argv[i];
			switch(arg) {
			case "-?":
			case "--help":
				printHelp();
				System.exit(0);
				break;
			case "-i":
			case "--interface":
				iface = argv[++i];
				break;
			case "-t":
			case "--timeout":
				try {
					timeout = Integer.parseInt(argv[++i]);
				}
				catch (NumberFormatException e) {
					System.err.println(argv[i-1] + "is not a valid number");
					System.exit(-1);
				}
				break;
			case "-q":
			case "--attempts":
				try {
					attempts = Integer.parseInt(argv[++i]);
				}
				catch (NumberFormatException e) {
					System.err.println(argv[i-1] + "is not a valid number");
					System.exit(-1);
				}
				break;
			case "-m":
			case "--hoplimit":
				try {
					hoplimit = Integer.parseInt(argv[++i]);
				}
				catch (NumberFormatException e) {
					System.err.println(argv[i-1] + "is not a valid number");
					System.exit(-1);
				}
				break;
			default:
				if(j == fargs.length) {
					System.out.println("Encountered an unexpected number of positional arguments");
					System.exit(1);
				}
				fargs[j++] = arg;
				break;
			}
		}
		if(fargs[0] == null) {
			System.out.println("Did not find positional argument: destination");
			System.exit(1);
		}
		dst = fargs[0];
	}
}
