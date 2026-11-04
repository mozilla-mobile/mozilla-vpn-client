// masque-vpn creates a TUN-based VPN that routes all traffic through MASQUE.
//
// The TUN device, system routing and userspace TCP/IP stack are provided by
// sing-tun (the TUN engine of sing-box). This binary only implements the
// tun.Handler that forwards every intercepted TCP connection / UDP session to
// the MASQUE relay (see handler.go) and the DNS hijack (see dns.go).
package main

import (
	"bufio"
	"context"
	"flag"
	"fmt"
	"log"
	"log/slog"
	"net"
	"net/netip"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	tun "github.com/sagernet/sing-tun"
	"github.com/sagernet/sing/common/control"
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

var logger *slog.Logger

const (
	mtu        = 1500
	udpTimeout = 5 * time.Minute
)

// resolveRelay resolves the relay hostname before the TUN comes up, so the
// lookup can't go through the tunnel. The result is used both to exclude the
// relay from the VPN routes and to answer the daemon's own later lookups of
// the relay name (see dnsForwarder.answerLocally).
func resolveRelay(host string) ([]netip.Addr, error) {
	if addr, err := netip.ParseAddr(host); err == nil {
		return []netip.Addr{addr.Unmap()}, nil
	}
	addrs, err := net.DefaultResolver.LookupNetIP(context.Background(), "ip", host)
	if err != nil {
		return nil, err
	}
	if len(addrs) == 0 {
		return nil, fmt.Errorf("no IP addresses found for %s", host)
	}
	for i := range addrs {
		addrs[i] = addrs[i].Unmap()
	}
	return addrs, nil
}

func buildTunOptions(relayIPs []netip.Addr) (tun.Options, error) {
	inet4, err := netip.ParsePrefix(fmt.Sprintf("%s/%d", *tunIP, *tunCIDR))
	if err != nil {
		return tun.Options{}, fmt.Errorf("invalid IPv4 address: %w", err)
	}

	options := tun.Options{
		Name:               *tunName,
		MTU:                mtu,
		Inet4Address:       []netip.Prefix{inet4},
		AutoRoute:          true,
		IPRoute2TableIndex: tun.DefaultIPRoute2TableIndex,
		IPRoute2RuleIndex:  tun.DefaultIPRoute2RuleIndex,
		InterfaceFinder:    control.NewDefaultInterfaceFinder(),
		Logger:             newSingLogger(logger),
	}

	if *tunIPv6 != "" {
		inet6, err := netip.ParsePrefix(fmt.Sprintf("%s/%d", *tunIPv6, *tunCIDRv6))
		if err != nil {
			return tun.Options{}, fmt.Errorf("invalid IPv6 address: %w", err)
		}
		options.Inet6Address = []netip.Prefix{inet6}
	}

	// CRITICAL: keep traffic to the MASQUE relay on the physical interface,
	// otherwise the tunnel would try to carry its own transport.
	for _, addr := range relayIPs {
		if addr.Is4() {
			options.Inet4RouteExcludeAddress = append(options.Inet4RouteExcludeAddress, netip.PrefixFrom(addr, 32))
		} else {
			options.Inet6RouteExcludeAddress = append(options.Inet6RouteExcludeAddress, netip.PrefixFrom(addr, 128))
		}
		logger.Info("Excluding relay from VPN routes", "ip", addr)
	}

	return options, nil
}

func connectMasque() (*MasqueClient, error) {
	var certData []byte
	if *certFile != "" {
		var err error
		certData, err = os.ReadFile(*certFile)
		if err != nil {
			return nil, err
		}
	}
	return NewClient(net.JoinHostPort(*relay, fmt.Sprint(*relayPort)), *token, certData, *insecure)
}

// readTokenLoop listens on stdin for rotated auth tokens, one token per line.
// When a new (non-empty) token arrives it is applied to the running client so
// that subsequently created TCP/UDP streams authenticate with the fresh token.
// Existing connections are deliberately left untouched, so rotation happens
// without interrupting ongoing traffic.
//
// Closing stdin (EOF) ends the loop but does NOT tear down the VPN: the tunnel
// keeps running with the last token it received.
func readTokenLoop(ctx context.Context, c *MasqueClient) {
	logger.Info("Listening for token rotations on stdin...")

	// Scan in a goroutine so a blocking Read on stdin can't prevent shutdown.
	lines := make(chan string)
	go func() {
		defer close(lines)
		scanner := bufio.NewScanner(os.Stdin)
		// Allow generously long tokens (default bufio limit is 64KiB).
		scanner.Buffer(make([]byte, 0, 64*1024), 1024*1024)
		for scanner.Scan() {
			lines <- scanner.Text()
		}
		if err := scanner.Err(); err != nil {
			logger.Error("stdin token reader error", "err", err)
		} else {
			logger.Info("stdin closed; token rotation listener stopped")
		}
	}()

	for {
		select {
		case <-ctx.Done():
			return
		case line, ok := <-lines:
			if !ok {
				return
			}
			newToken := strings.TrimSpace(line)
			if newToken == "" {
				continue
			}
			if newToken == c.AuthToken() {
				logger.Debug("Received token identical to current one; ignoring")
				continue
			}
			c.SetAuthToken(newToken)
			logger.Info("Auth token rotated; new streams will use the refreshed token")
		}
	}
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
	dnsServer = flag.String("dns", "8.8.8.8", "Upstream DNS server, reached through MASQUE")

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

	// Root check
	if os.Geteuid() != 0 {
		log.Fatal("Must run as root")
	}

	// CRITICAL: Resolve relay hostname BEFORE creating TUN to avoid DNS through VPN
	logger.Info("Resolving relay hostname", "relay", *relay)
	relayIPs, err := resolveRelay(*relay)
	if err != nil {
		log.Fatalf("Failed to resolve relay hostname %s: %v", *relay, err)
	}
	logger.Info("Resolved relay IPs", "ips", relayIPs)

	client, err := connectMasque()
	if err != nil {
		log.Fatalf("MASQUE client setup failed: %v", err)
	}
	defer client.Close()

	upstreamDNS, err := netip.ParseAddr(*dnsServer)
	if err != nil {
		log.Fatalf("Invalid DNS server %q: %v", *dnsServer, err)
	}

	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()

	handler := &masqueHandler{
		client: client,
		dns:    newDNSForwarder(client, netip.AddrPortFrom(upstreamDNS, 53), *relay, relayIPs),
	}
	defer handler.dns.Close()

	tunOptions, err := buildTunOptions(relayIPs)
	if err != nil {
		log.Fatalf("Invalid TUN configuration: %v", err)
	}

	// The Linux TUN registers itself with the monitor on Start, so it is
	// required even though we don't react to interface changes.
	networkMonitor, err := tun.NewNetworkUpdateMonitor(tunOptions.Logger)
	if err != nil {
		log.Fatalf("Network monitor creation failed: %v", err)
	}
	interfaceMonitor, err := tun.NewDefaultInterfaceMonitor(networkMonitor, tunOptions.Logger, tun.DefaultInterfaceMonitorOptions{
		InterfaceFinder: tunOptions.InterfaceFinder,
	})
	if err != nil {
		log.Fatalf("Interface monitor creation failed: %v", err)
	}
	if err := networkMonitor.Start(); err != nil {
		log.Fatalf("Network monitor start failed: %v", err)
	}
	defer networkMonitor.Close()
	if err := interfaceMonitor.Start(); err != nil {
		log.Fatalf("Interface monitor start failed: %v", err)
	}
	defer interfaceMonitor.Close()
	tunOptions.InterfaceMonitor = interfaceMonitor

	tunInterface, err := tun.New(tunOptions)
	if err != nil {
		log.Fatalf("TUN creation failed: %v", err)
	}
	defer tunInterface.Close()

	// Only the system stack: handler.go relies on its packet writer leaving
	// buffers to the caller (the gvisor one releases them itself).
	tunStack, err := tun.NewStack("system", tun.StackOptions{
		Context:         ctx,
		Tun:             tunInterface,
		TunOptions:      tunOptions,
		UDPTimeout:      udpTimeout,
		Handler:         handler,
		Logger:          tunOptions.Logger,
		InterfaceFinder: tunOptions.InterfaceFinder,
	})
	if err != nil {
		log.Fatalf("Stack creation failed: %v", err)
	}
	defer tunStack.Close()

	if err := tunStack.Start(); err != nil {
		log.Fatalf("Stack start failed: %v", err)
	}
	if err := tunInterface.Start(); err != nil {
		log.Fatalf("TUN start failed: %v", err)
	}

	// Listen for rotated auth tokens on stdin. The tunnel stays up even after
	// stdin is closed; the loop exits on ctx cancellation during shutdown.
	go readTokenLoop(ctx, client)

	name, _ := tunInterface.Name()
	logger.Info("MASQUE VPN started",
		"tun", name,
		"ipv4", tunOptions.Inet4Address,
		"ipv6", tunOptions.Inet6Address,
		"relay", net.JoinHostPort(*relay, fmt.Sprint(*relayPort)),
		"dns", *dnsServer)

	// Wait for a shutdown signal; deferred Close calls tear down the stack,
	// routes and TUN interface.
	sigCh := make(chan os.Signal, 1)
	signal.Notify(sigCh, syscall.SIGINT, syscall.SIGTERM)
	<-sigCh
	logger.Info("Shutting down...")
}
