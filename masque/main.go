// masque-vpn creates a TUN-based VPN that routes all traffic through MASQUE
// using gvisor's netstack as the userspace TCP/IP stack.
package main

import (
	"context"
	"flag"
	"fmt"
	"io"
	"log"
	"log/slog"
	"net"
	"os"
	"os/exec"
	"os/signal"
	"sync"
	"syscall"
	"time"

	"github.com/songgao/water"
	"gvisor.dev/gvisor/pkg/buffer"
	"gvisor.dev/gvisor/pkg/tcpip"
	"gvisor.dev/gvisor/pkg/tcpip/adapters/gonet"
	"gvisor.dev/gvisor/pkg/tcpip/header"
	"gvisor.dev/gvisor/pkg/tcpip/link/channel"
	"gvisor.dev/gvisor/pkg/tcpip/network/ipv4"
	"gvisor.dev/gvisor/pkg/tcpip/network/ipv6"
	"gvisor.dev/gvisor/pkg/tcpip/stack"
	"gvisor.dev/gvisor/pkg/tcpip/transport/tcp"
	"gvisor.dev/gvisor/pkg/tcpip/transport/udp"
	"gvisor.dev/gvisor/pkg/waiter"
)

// Command line flags
var (
	relay     *string
	relayPort *int
	token     *string
	insecure  *bool
	certFile  *string
	verbose   *bool
	tunName   *string
	tunIP     *string
	tunCIDR   *int
	tunIPv6   *string
	tunCIDRv6 *int
	dnsServer *string
)

var (
	client *MasqueClient
	logger *slog.Logger
	
	// Semaphore to limit concurrent UDP stream creations
	// This prevents overwhelming the QUIC connection during startup
	udpStreamSemaphore chan struct{}
)

const (
	nicID                   = 1
	mtu                     = 1500
	bufferMB                = 4
	maxConcurrentUDPStreams = 20 // Higher limit since each stream has its own connection
)

var defaultInterface string
var defaultGateway string

// TUN device wrapper
type tunDevice struct {
	iface    *water.Interface
	mtu      int
	endpoint *channel.Endpoint
}

// getDefaultInterface returns the name of the default network interface
func getDefaultInterface() (string, error) {
	// Get default route to determine the default interface
	cmd := exec.Command("ip", "route", "show", "default")
	output, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("failed to get default route: %w", err)
	}

	// Parse output like: "default via 192.168.1.1 dev eth0 proto dhcp metric 100"
	// Simple parsing: find "dev" and take the next word
	outputStr := string(output)
	parts := []rune(outputStr)
	
	// Find "dev " in the output
	devIdx := -1
	for i := 0; i < len(outputStr)-4; i++ {
		if outputStr[i:i+4] == "dev " {
			devIdx = i + 4
			break
		}
	}
	
	if devIdx >= 0 {
		// Extract interface name (up to space or newline)
		iface := ""
		for i := devIdx; i < len(parts); i++ {
			if parts[i] == ' ' || parts[i] == '\n' || parts[i] == '\t' {
				break
			}
			iface += string(parts[i])
		}
		if iface != "" {
			return iface, nil
		}
	}

	return "", fmt.Errorf("could not determine default interface from: %s", outputStr)
}

// getDefaultGateway returns the IP address of the default gateway
func getDefaultGateway() (string, error) {
	cmd := exec.Command("ip", "route", "show", "default")
	output, err := cmd.Output()
	if err != nil {
		return "", fmt.Errorf("failed to get default route: %w", err)
	}

	// Parse output like: "default via 192.168.1.1 dev eth0 proto dhcp metric 100"
	// Find "via " and take the next word (the gateway IP)
	outputStr := string(output)
	parts := []rune(outputStr)
	
	viaIdx := -1
	for i := 0; i < len(outputStr)-4; i++ {
		if outputStr[i:i+4] == "via " {
			viaIdx = i + 4
			break
		}
	}
	
	if viaIdx >= 0 {
		gateway := ""
		for i := viaIdx; i < len(parts); i++ {
			if parts[i] == ' ' || parts[i] == '\n' || parts[i] == '\t' {
				break
			}
			gateway += string(parts[i])
		}
		if gateway != "" {
			return gateway, nil
		}
	}

	return "", fmt.Errorf("could not determine default gateway from: %s", outputStr)
}

func newTunDevice(name string, relayIPs []string) (*tunDevice, error) {
	config := water.Config{
		DeviceType: water.TUN,
	}
	if name != "" {
		config.Name = name
	}

	iface, err := water.New(config)
	if err != nil {
		return nil, fmt.Errorf("create TUN: %w", err)
	}

	logger.Info("TUN interface created", "name", iface.Name())

	// Configure interface
	cmds := [][]string{
		{"ip", "link", "set", "dev", iface.Name(), "mtu", fmt.Sprint(mtu)},
		{"ip", "addr", "add", fmt.Sprintf("%s/%d", *tunIP, *tunCIDR), "dev", iface.Name()},
		{"ip", "link", "set", "dev", iface.Name(), "up"},
	}

	// Add IPv6 address if provided
	if *tunIPv6 != "" {
		cmds = append(cmds, []string{"ip", "-6", "addr", "add", fmt.Sprintf("%s/%d", *tunIPv6, *tunCIDRv6), "dev", iface.Name()})
	}

	// CRITICAL: Add relay server routes FIRST before default route to prevent routing loops
	// These routes ensure traffic to the MASQUE relay goes through the physical interface
	if defaultInterface != "" && defaultGateway != "" {
		for _, addr := range relayIPs {
			ip := net.ParseIP(addr)
			if ip != nil {
				if ip.To4() != nil {
					// IPv4 address - route via gateway
					cmds = append(cmds, []string{"ip", "route", "replace", addr + "/32", "via", defaultGateway, "dev", defaultInterface})
					logger.Info("Adding relay route", "ip", addr, "via", defaultGateway, "dev", defaultInterface)
				} else {
					// IPv6 address - route via gateway (need to get IPv6 gateway separately)
					// For now, just use dev without gateway for IPv6
					cmds = append(cmds, []string{"ip", "-6", "route", "replace", addr + "/128", "dev", defaultInterface})
					logger.Info("Adding relay route (IPv6)", "ip", addr, "dev", defaultInterface)
				}
			}
		}
	}

	// Add DNS server routes through default interface to prevent routing loops
	if defaultInterface != "" && defaultGateway != "" {
		cmds = append(cmds,
			[]string{"ip", "route", "replace", "8.8.8.8/32", "via", defaultGateway, "dev", defaultInterface},
			[]string{"ip", "route", "replace", "1.1.1.1/32", "via", defaultGateway, "dev", defaultInterface},
			[]string{"ip", "route", "replace", "9.9.9.9/32", "via", defaultGateway, "dev", defaultInterface},
		)
	}

	// Add default routes through TUN with metrics to control preference
	// Lower metric = higher priority
	// IPv6 gets metric 50 (high priority) - will be preferred for dual-stack destinations
	// IPv4 gets metric 500 (low priority) - only used for IPv4-only destinations
	if *tunIPv6 != "" {
		cmds = append(cmds, []string{"ip", "-6", "route", "replace", "::/0", "dev", iface.Name(), "metric", "50"})
		logger.Info("IPv6 default route added through VPN with metric 50 (high priority)")
	}
	cmds = append(cmds, []string{"ip", "route", "replace", "0.0.0.0/0", "dev", iface.Name(), "metric", "500"})
	logger.Info("IPv4 default route added through VPN with metric 500 (low priority)")

	for _, cmd := range cmds {
		logger.Debug("Running command", "cmd", cmd)
		if err := exec.Command(cmd[0], cmd[1:]...).Run(); err != nil {
			return nil, fmt.Errorf("exec %v: %w", cmd, err)
		}
	}
	logger.Info("TUN interface configured")

	logger.Info("TUN interface created", "name", iface.Name(), "ip", *tunIP)

	endpoint := channel.New(512, uint32(mtu), "")

	return &tunDevice{
		iface:    iface,
		mtu:      mtu,
		endpoint: endpoint,
	}, nil
}

// Read from TUN and inject into netstack
func (t *tunDevice) readLoop(ctx context.Context) {
	logger.Info("Starting read loop from TUN...")
	buf := make([]byte, mtu+14) // Extra space for ethernet header if needed

	for {
		select {
		case <-ctx.Done():
			return
		default:
		}

		n, err := t.iface.Read(buf)
		if err != nil {
			logger.Error("TUN read error", "err", err)
			continue
		}

		if n == 0 {
			continue
		}

		pkt := buf[:n]

		// Determine IP version
		version := pkt[0] >> 4
		var proto tcpip.NetworkProtocolNumber
		
		if version == 4 {
			if len(pkt) < 20 {
				continue
			}
			proto = header.IPv4ProtocolNumber
			// Parse basic IPv4 info for logging
			srcIP := net.IP(pkt[12:16])
			dstIP := net.IP(pkt[16:20])
			protocol := pkt[9]
			logger.Debug("IPv4 packet", "src", srcIP, "dst", dstIP, "proto", protocol, "len", n)
		} else if version == 6 {
			if len(pkt) < 40 {
				continue
			}
			proto = header.IPv6ProtocolNumber
			srcIP := net.IP(pkt[8:24])
			dstIP := net.IP(pkt[24:40])
			logger.Debug("IPv6 packet", "src", srcIP, "dst", dstIP, "len", n)
		} else {
			logger.Debug("Unknown IP version", "version", version)
			continue
		}

		// Create packet buffer
		vv := make([]byte, n)
		copy(vv, pkt)

		pbuf := stack.NewPacketBuffer(stack.PacketBufferOptions{
			Payload: buffer.MakeWithData(vv),
		})

		logger.Debug("Injecting packet into netstack", "proto", proto, "size", n)
		t.endpoint.InjectInbound(proto, pbuf)
		pbuf.DecRef()
	}
}

// Read from netstack and write to TUN
func (t *tunDevice) writeLoop(ctx context.Context) {
	logger.Info("Starting write loop to TUN...")
	for {
		select {
		case <-ctx.Done():
			return
		default:
		}

		pkt := t.endpoint.ReadContext(ctx)
		if pkt == nil {
			return
		}

		// Get packet as contiguous bytes
		view := pkt.ToView()
		buf := view.AsSlice()
		
		if len(buf) > 0 {
			logger.Debug("Writing packet to TUN", "size", len(buf))
			if _, err := t.iface.Write(buf); err != nil {
				logger.Error("TUN write error", "err", err)
			}
		}
		
		pkt.DecRef()
	}
}

// Handle TCP connection by forwarding through MASQUE
func handleTCPConn(wq *waiter.Queue, ep tcpip.Endpoint, conn *gonet.TCPConn, reqID stack.TransportEndpointID) {
	defer conn.Close()

	// reqID.LocalAddress/LocalPort = DESTINATION (where packet was going)
	// reqID.RemoteAddress/RemotePort = SOURCE (where packet came from)
	
	// Format target address correctly for IPv6 (wrap in brackets)
	var target string
	destAddr := reqID.LocalAddress.String()
	if net.ParseIP(destAddr).To4() == nil && net.ParseIP(destAddr) != nil {
		// IPv6 address - wrap in brackets
		target = fmt.Sprintf("[%s]:%d", destAddr, reqID.LocalPort)
	} else {
		// IPv4 address
		target = fmt.Sprintf("%s:%d", destAddr, reqID.LocalPort)
	}
	
	// Format source for logging
	var source string
	srcAddr := reqID.RemoteAddress.String()
	if net.ParseIP(srcAddr).To4() == nil && net.ParseIP(srcAddr) != nil {
		source = fmt.Sprintf("[%s]:%d", srcAddr, reqID.RemotePort)
	} else {
		source = fmt.Sprintf("%s:%d", srcAddr, reqID.RemotePort)
	}
	
	logger.Info("*** NEW TCP CONNECTION ***", "target", target, "source", source)
	logger.Debug("New TCP connection", "target", target)

	// Check port
	if IsDisallowedPort(reqID.LocalPort) {
		logger.Warn("Blocked port", "port", reqID.LocalPort)
		return
	}

	// Create MASQUE stream
	masqueConn, err := client.CreateTCPStream(target)
	if err != nil {
		logger.Error("MASQUE create failed", "err", err, "target", target)
		return
	}
	defer masqueConn.Close()

	logger.Info("Forwarding connection", "target", target)

	// Bidirectional copy
	var wg sync.WaitGroup
	wg.Add(2)

	go func() {
		defer wg.Done()
		io.Copy(masqueConn, conn)
	}()

	go func() {
		defer wg.Done()
		io.Copy(conn, masqueConn)
	}()

	wg.Wait()
	logger.Debug("Connection closed", "target", target)
}

// TCP forwarder
func setupTCPForwarder(s *stack.Stack) {
	logger.Info("Setting up TCP forwarder...")
	fwd := tcp.NewForwarder(s, 0, 10000, func(r *tcp.ForwarderRequest) {
		// Extract ID before calling Complete() - r becomes invalid after Complete()
		reqID := r.ID()
		logger.Info("*** FORWARDER RECEIVED REQUEST ***", "id", reqID)
		
		var wq waiter.Queue
		ep, err := r.CreateEndpoint(&wq)
		if err != nil {
			logger.Error("Create endpoint failed", "err", err)
			r.Complete(true)
			return
		}
		r.Complete(false)
		logger.Info("Endpoint created, starting handler")

		conn := gonet.NewTCPConn(&wq, ep)
		go handleTCPConn(&wq, ep, conn, reqID)
	})

	s.SetTransportProtocolHandler(tcp.ProtocolNumber, fwd.HandlePacket)
	logger.Info("TCP forwarder installed")
}

// Handle UDP packet by forwarding through MASQUE
func handleUDPPacket(s *stack.Stack, r *udp.ForwarderRequest) bool {
	reqID := r.ID()
	
	// Format target address correctly for IPv6
	var target string
	destAddr := reqID.LocalAddress.String()
	if net.ParseIP(destAddr).To4() == nil && net.ParseIP(destAddr) != nil {
		// IPv6 address - wrap in brackets
		target = fmt.Sprintf("[%s]:%d", destAddr, reqID.LocalPort)
	} else {
		// IPv4 address
		target = fmt.Sprintf("%s:%d", destAddr, reqID.LocalPort)
	}
	
	// Format source for logging
	var source string
	srcAddr := reqID.RemoteAddress.String()
	if net.ParseIP(srcAddr).To4() == nil && net.ParseIP(srcAddr) != nil {
		source = fmt.Sprintf("[%s]:%d", srcAddr, reqID.RemotePort)
	} else {
		source = fmt.Sprintf("%s:%d", srcAddr, reqID.RemotePort)
	}
	
	logger.Info("*** NEW UDP PACKET ***", "target", target, "source", source)
	
	// Check port
	/*if IsDisallowedPort(reqID.LocalPort) {
		logger.Warn("Blocked UDP port", "port", reqID.LocalPort)
		return false
	}*/
	
	// Acquire semaphore to limit concurrent UDP stream creations
	// This prevents overwhelming the connection during startup
	udpStreamSemaphore <- struct{}{}
	defer func() { <-udpStreamSemaphore }()
	
	// Create MASQUE UDP stream
	masqueConn, err := client.CreateUDPStream(target)
	if err != nil {
		logger.Error("MASQUE UDP create failed", "err", err, "target", target)
		return false
	}
	defer masqueConn.Close()
	
	logger.Info("Forwarding UDP", "target", target)
	
	// Create endpoint for UDP
	var wq waiter.Queue
	ep, tcpipErr := r.CreateEndpoint(&wq)
	if tcpipErr != nil {
		logger.Error("UDP create endpoint failed", "err", tcpipErr.String())
		return false
	}
	
	udpConn := gonet.NewUDPConn(&wq, ep)
	defer udpConn.Close()
	
	// Bidirectional copy for UDP
	var wgUDP sync.WaitGroup
	wgUDP.Add(2)
	
	go func() {
		defer wgUDP.Done()
		io.Copy(masqueConn, udpConn)
	}()
	
	go func() {
		defer wgUDP.Done()
		io.Copy(udpConn, masqueConn)
	}()
	
	wgUDP.Wait()
	logger.Debug("UDP connection closed", "target", target)
	return true
}

// UDP forwarder
func setupUDPForwarder(s *stack.Stack) {
	logger.Info("Setting up UDP forwarder...")
	fwd := udp.NewForwarder(s, func(r *udp.ForwarderRequest) bool {
		go handleUDPPacket(s, r)
		return true
	})
	
	s.SetTransportProtocolHandler(udp.ProtocolNumber, fwd.HandlePacket)
	logger.Info("UDP forwarder installed")
}

// Setup routing
func setupRouting(tunName string) error {
	if *dnsServer != "" {
		// Backup and set DNS
		exec.Command("cp", "/etc/resolv.conf", "/etc/resolv.conf.backup").Run()
		os.WriteFile("/etc/resolv.conf", []byte("nameserver "+*dnsServer+"\n"), 0644)
	}

	logger.Info("Routing configured")
	return nil
}

func cleanup(tunName string) {
	// Restore DNS
	if *dnsServer != "" {
		exec.Command("mv", "/etc/resolv.conf.backup", "/etc/resolv.conf").Run()
	}

	var cmds [][]string

	// Remove relay routes
	if defaultInterface != "" {
		addrs, err := net.LookupHost(*relay)
		if err == nil {
			for _, addr := range addrs {
				ip := net.ParseIP(addr)
				if ip != nil {
					if ip.To4() != nil {
						cmds = append(cmds, []string{"ip", "route", "del", addr + "/32", "via", defaultGateway, "dev", defaultInterface})
					} else {
						cmds = append(cmds, []string{"ip", "-6", "route", "del", addr + "/128", "via", defaultGateway, "dev", defaultInterface})
					}
				}
			}
		}
	}

	// Remove DNS routes
	if defaultInterface != "" {
		cmds = append(cmds,
			[]string{"ip", "route", "del", "8.8.8.8/32", "via", defaultGateway, "dev", defaultInterface},
			[]string{"ip", "route", "del", "1.1.1.1/32", "via", defaultGateway, "dev", defaultInterface},
			[]string{"ip", "route", "del", "9.9.9.9/32", "via", defaultGateway, "dev", defaultInterface},
		)
	}

	// Remove default routes
	if *tunIPv6 != "" {
		cmds = append(cmds, []string{"ip", "-6", "route", "del", "::/0", "dev", tunName, "metric", "50"})
	}
	cmds = append(cmds, []string{"ip", "route", "del", "0.0.0.0/0", "dev", tunName, "metric", "500"})



	for _, cmd := range cmds {
		logger.Debug("Running command", "cmd", cmd)
		exec.Command(cmd[0], cmd[1:]...).Run()
	}
	logger.Info("Cleanup done")
}

func connectMasque() (*MasqueClient, error) {
	var certData []byte
	var err error
	if *certFile != "" {
		certData, err = os.ReadFile(*certFile)
		if err != nil {
			return nil, err
		}
	}

	cfg := ClientConfig{
		ProxyAddr:  fmt.Sprintf("%s:%d", *relay, *relayPort),
		AuthToken:  *token,
		Logger:     logger,
		CertData:   certData,
		IgnoreCert: *insecure,
	}

	c, err := NewClient(cfg)
	if err != nil {
		return nil, fmt.Errorf("create client: %w", err)
	}

	if err := c.ConnectToProxy(); err != nil {
		return nil, err
	}

	return c, nil
}

func main() {
	relay = flag.String("relay", "", "MASQUE relay server")
	relayPort = flag.Int("port", 443, "Relay port")
	token = flag.String("token", "", "Auth token")
	insecure = flag.Bool("insecure", false, "Skip TLS verify")
	certFile = flag.String("cert", "", "Certificate file")
	verbose = flag.Bool("verbose", false, "Debug logging")
	tunName = flag.String("tun", "masque0", "TUN interface name")
	tunIP = flag.String("ip", "10.0.0.1", "TUN IPv4 address")
	tunCIDR = flag.Int("cidr", 24, "IPv4 network CIDR prefix")
	tunIPv6 = flag.String("ipv6", "fd00::1", "TUN IPv6 address (empty to disable)")
	tunCIDRv6 = flag.Int("cidr6", 64, "IPv6 network CIDR prefix")
	dnsServer = flag.String("dns", "8.8.8.8", "DNS server")

	flag.Parse()

	if *relay == "" || *token == "" {
		fmt.Println("Required: -relay and -token")
		flag.Usage()
		os.Exit(1)
	}

	// Logging
	level := slog.LevelInfo
	if *verbose {
		level = slog.LevelDebug
	}
	logger = slog.New(slog.NewTextHandler(os.Stdout, &slog.HandlerOptions{Level: level}))
	slog.SetDefault(logger)

	logger.Info("MASQUE VPN starting...", "verbose", *verbose, "logLevel", level)
	
	// Initialize UDP stream semaphore to limit concurrent creations
	udpStreamSemaphore = make(chan struct{}, maxConcurrentUDPStreams)
	
	// Root check
	if os.Geteuid() != 0 {
		log.Fatal("Must run as root")
	}

	// Detect default network interface and gateway
	var err error
	defaultInterface, err = getDefaultInterface()
	if err != nil {
		logger.Warn("Could not detect default interface", "err", err)
		logger.Warn("Relay routing may not work correctly")
	} else {
		logger.Info("Detected default interface", "interface", defaultInterface)
	}

	defaultGateway, err = getDefaultGateway()
	if err != nil {
		logger.Warn("Could not detect default gateway", "err", err)
		logger.Warn("Relay routing may not work correctly")
	} else {
		logger.Info("Detected default gateway", "gateway", defaultGateway)
	}

	// CRITICAL: Resolve relay hostname BEFORE creating TUN to avoid DNS through VPN
	logger.Info("Resolving relay hostname", "relay", *relay)
	relayIPs, err := net.LookupHost(*relay)
	if err != nil {
		log.Fatalf("Failed to resolve relay hostname %s: %v", *relay, err)
	}
	if len(relayIPs) == 0 {
		log.Fatalf("No IP addresses found for relay %s", *relay)
	}
	logger.Info("Resolved relay IPs", "ips", relayIPs)

	// Connect to MASQUE
	logger.Info("Connecting to MASQUE", "relay", *relay)
	client, err = connectMasque()
	if err != nil {
		log.Fatalf("MASQUE connection failed: %v", err)
	}
	logger.Info("MASQUE connected")

	// Create TUN with relay IPs for proper routing
	tun, err := newTunDevice(*tunName, relayIPs)
	if err != nil {
		log.Fatalf("TUN creation failed: %v", err)
	}
	defer cleanup(*tunName)

	// Create netstack
	logger.Info("Creating netstack...")
	s := stack.New(stack.Options{
		NetworkProtocols:   []stack.NetworkProtocolFactory{ipv4.NewProtocol, ipv6.NewProtocol},
		TransportProtocols: []stack.TransportProtocolFactory{tcp.NewProtocol, udp.NewProtocol},
	})

	if err := s.CreateNIC(nicID, tun.endpoint); err != nil {
		log.Fatalf("CreateNIC failed: %v", err)
	}

	// Enable promiscuous mode and spoofing to accept all packets
	if err := s.SetPromiscuousMode(nicID, true); err != nil {
		logger.Warn("SetPromiscuousMode failed", "err", err)
	}
	if err := s.SetSpoofing(nicID, true); err != nil {
		logger.Warn("SetSpoofing failed", "err", err)
	}
	logger.Info("NIC created with promiscuous mode")

	// Configure IPv4 address for the TUN interface itself
	ipBytes := net.ParseIP(*tunIP).To4()
	if ipBytes != nil {
		addr := tcpip.AddrFrom4([4]byte{ipBytes[0], ipBytes[1], ipBytes[2], ipBytes[3]})
		protoAddr := tcpip.ProtocolAddress{
			Protocol:          ipv4.ProtocolNumber,
			AddressWithPrefix: tcpip.AddressWithPrefix{Address: addr, PrefixLen: *tunCIDR},
		}

		if err := s.AddProtocolAddress(nicID, protoAddr, stack.AddressProperties{}); err != nil {
			log.Fatalf("AddProtocolAddress failed: %v", err)
		}
	}

	// Add a catch-all subnet (0.0.0.0/0) so netstack processes packets to ANY destination
	// This makes netstack think it's responsible for the entire internet, enabling transparent proxying
	if err := s.AddProtocolAddress(nicID, tcpip.ProtocolAddress{
		Protocol: ipv4.ProtocolNumber,
		AddressWithPrefix: tcpip.AddressWithPrefix{
			Address:   tcpip.AddrFrom4([4]byte{0, 0, 0, 0}),
			PrefixLen: 0, // Catches everything
		},
	}, stack.AddressProperties{}); err != nil {
		logger.Warn("Failed to add catch-all IPv4 subnet", "err", err)
	}
	logger.Info("Added catch-all IPv4 subnet for transparent proxying")

	// Configure IPv6 addresses
	if *tunIPv6 != "" {
		// Add configured IPv6 address
		ipv6Bytes := net.ParseIP(*tunIPv6)
		if ipv6Bytes != nil && ipv6Bytes.To4() == nil {
			var addr16 [16]byte
			copy(addr16[:], ipv6Bytes)
			addr := tcpip.AddrFrom16(addr16)
			protoAddr := tcpip.ProtocolAddress{
				Protocol:          ipv6.ProtocolNumber,
				AddressWithPrefix: tcpip.AddressWithPrefix{Address: addr, PrefixLen: *tunCIDRv6},
			}
			if err := s.AddProtocolAddress(nicID, protoAddr, stack.AddressProperties{}); err != nil {
				logger.Warn("Failed to add configured IPv6 address", "err", err)
			} else {
				logger.Info("Added IPv6 address", "address", *tunIPv6, "prefix", *tunCIDRv6)
			}
		}
	}

	// Always add link-local address
	if err := s.AddProtocolAddress(nicID, tcpip.ProtocolAddress{
		Protocol: ipv6.ProtocolNumber,
		AddressWithPrefix: tcpip.AddressWithPrefix{
			Address:   tcpip.AddrFrom16([16]byte{0xfe, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}),
			PrefixLen: 64,
		},
	}, stack.AddressProperties{}); err != nil {
		logger.Warn("Failed to add IPv6 link-local address", "err", err)
	}

	// Add IPv6 catch-all for transparent proxying
	if err := s.AddProtocolAddress(nicID, tcpip.ProtocolAddress{
		Protocol: ipv6.ProtocolNumber,
		AddressWithPrefix: tcpip.AddressWithPrefix{
			Address:   tcpip.AddrFrom16([16]byte{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}),
			PrefixLen: 0,
		},
	}, stack.AddressProperties{}); err != nil {
		logger.Warn("Failed to add catch-all IPv6 subnet", "err", err)
	}
	logger.Info("Added catch-all IPv6 subnet for transparent proxying")

	// Route table - support both IPv4 and IPv6
	s.SetRouteTable([]tcpip.Route{
		{
			Destination: header.IPv4EmptySubnet,
			NIC:         nicID,
		},
		{
			Destination: header.IPv6EmptySubnet,
			NIC:         nicID,
		},
	})

	// DO NOT enable forwarding - we want netstack to process packets locally, not forward them
	// s.SetForwardingDefaultAndAllNICs() would make netstack act as a router instead of an endpoint
	logger.Info("Routes configured (forwarding disabled for local processing)")

	// Setup TCP and UDP forwarders
	setupTCPForwarder(s)
	setupUDPForwarder(s)

	logger.Info("Netstack ready")

	// Context for shutdown
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	// Signal handling
	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, syscall.SIGINT, syscall.SIGTERM)
	go func() {
		<-sigCh
		logger.Info("Shutting down...")
		cancel()
	}()

	// Start TUN loops
	var wg sync.WaitGroup
	wg.Add(2)

	go func() {
		defer wg.Done()
		tun.readLoop(ctx)
	}()

	go func() {
		defer wg.Done()
		tun.writeLoop(ctx)
	}()

	// Setup system routing
	if err := setupRouting(tun.iface.Name()); err != nil {
		logger.Warn("Routing setup failed", "err", err)
	}

	fmt.Printf("\n")
	fmt.Printf("╔════════════════════════════════════════════════════════╗\n")
	fmt.Printf("║  MASQUE VPN Started                                    ║\n")
	fmt.Printf("╠════════════════════════════════════════════════════════╣\n")
	fmt.Printf("║  TUN Interface: %-35s║\n", tun.iface.Name())
	fmt.Printf("║  IPv4 Address:  %-35s║\n", fmt.Sprintf("%s/%d", *tunIP, *tunCIDR))
	if *tunIPv6 != "" {
		fmt.Printf("║  IPv6 Address:  %-35s║\n", fmt.Sprintf("%s/%d", *tunIPv6, *tunCIDRv6))
	}
	fmt.Printf("║  MASQUE Relay:  %-35s║\n", fmt.Sprintf("%s:%d", *relay, *relayPort))
	if defaultInterface != "" {
		fmt.Printf("║  Phys Interface:%-35s║\n", defaultInterface)
	}
	fmt.Printf("║  DNS Server:    %-35s║\n", *dnsServer)
	fmt.Printf("╠════════════════════════════════════════════════════════╣\n")
	fmt.Printf("║  All traffic is now routed through MASQUE              ║\n")
	fmt.Printf("║  Press Ctrl+C to stop                                  ║\n")
	fmt.Printf("╚════════════════════════════════════════════════════════╝\n")
	fmt.Printf("\n")

	// Wait
	wg.Wait()

	// Cleanup
	time.Sleep(time.Second)
	logger.Info("VPN stopped")
}
