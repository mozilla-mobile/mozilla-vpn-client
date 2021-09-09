# Mozilla VPN - IPMonitor

IPMonitor is a simple libpcap app to see which packets are sent and received
outside the VPN tunnel. This is meant to be used as a debugging/testing tool
and it's not included in the MozillaVPN app bundle.

### How to compile it

You must have the pcap library: https://www.tcpdump.org/

On debian/ubuntu: `apt-get install libpcap0.8-dev`
On macos: `brew reinstall libpcap`

After this, run the following commands:

1. [for macOn + brew only] `export PKG_CONFIG_PATH=/usr/local/opt/libpcap/lib/pkgconfig`
2. `qmake && make`

### How to fetch the list of servers from the MozillaVPN app

First step is to retrieve the list of servers. You must have the mozillavpn
client up and running. Then, run the following command:

`mozillavpn servers -j > servers.json`

Use `servers.json` as argument for the ipmonitor app.

### How to run the app

The app has an help menu:

```
$ ./ipmonitor -h 
Usage: ./ipmonitor [options] server.list
Mozilla VPN - IP Monitor

Options:
  -h, --help     Displays help on commandline options.
  --help-all     Displays help including Qt specific options.
  -v, --version  Displays version information.
  -l <file>      Log non-VPN and non-LAN packets into this file

Arguments:
  server.list    The server list file
```

To run the app, pass the `server.json` as first argument. We use `sudo` to have
the permissions to open the network interfaces.

```
$ sudo ./ipmonitor server.list
make: Nothing to be done for `first'.
Mozilla VPN - IP Monitor 0.1

Processing the list of servers: 420 servers.
Retrieving the interface list: done.
The interfaces present on the system are:
 1 ap1 - 
 2 en0 - 
 3 awdl0 - 
 4 llw0 - 
 5 utun0 - 
 6 utun1 - 
 7 en5 - 
 8 en3 - 
 9 en4 - 
 10 en1 - 
 11 en2 - 
 12 bridge0 - 
Which interface do you want to use?
```

At this point you have to choose the network interface you want to monitor.
`en0` for MacOS and a `wlp*` interface on linux could be a good starting point.

The app has this simple UI:
```
Total: 832 - VPN: 539 - LAN: 69 - OUT: 224
D: < - Src:   192.168.0.128 - Dest:   193.32.249.69
Loc: Netherlands, Amsterdam
```

Where:
 - Total: the total number of packets received + sent.
 - VPN: the packets via the VPN tunnel interface
 - LAN: the packets for your local-area-network
 - OUT: the packets outside the VPN tunnel interface
 - D: the direction. It can be '>', for a sent packet, or '<', for a receved one.
 - Src: the source IP address
 - Dest: the destination IP address
 - Loc: the VPN location, if detected

### How to log the result

Use `-l <fileName>` to write some data related to non-VPN and non-LAN packets.
This part needs to be improved.
