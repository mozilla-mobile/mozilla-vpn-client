// MASQUE client implementation for proxying TCP connections over HTTP/2
// and UDP connections over HTTP/3
package main

import (
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io"
	"log/slog"
	"net"
	"net/http"
	"strings"
	"time"

	masquehttp3 "github.com/invisv-privacy/masque/http3"
	"golang.org/x/net/http2"
)

// MasqueClient implements MASQUE protocol over HTTP/2 and HTTP/3
type MasqueClient struct {
	proxyAddr   string
	proxyHost   string // hostname extracted from proxyAddr for TLS SNI
	authToken   string
	tlsConfig   *tls.Config
	logger      *slog.Logger
	h2Transport *http2.Transport
	
	// masque HTTP/3 client for UDP streams
	masqueH3Client *masquehttp3.Client
}

// ClientConfig holds configuration for MASQUE client
type ClientConfig struct {
	ProxyAddr  string
	AuthToken  string
	Logger     *slog.Logger
	CertData   []byte
	IgnoreCert bool
}

// NewClient creates a new MASQUE client
func NewClient(cfg ClientConfig) (*MasqueClient, error) {
	tlsConfig := &tls.Config{
		InsecureSkipVerify: cfg.IgnoreCert,
		NextProtos:         []string{"h2"},
	}

	// Add custom CA certificate if provided
	if len(cfg.CertData) > 0 {
		certPool := x509.NewCertPool()
		if certPool.AppendCertsFromPEM(cfg.CertData) {
			tlsConfig.RootCAs = certPool
		}
	}

	// Extract hostname from proxy address for TLS SNI
	proxyHost, _, err := net.SplitHostPort(cfg.ProxyAddr)
	if err != nil {
		// If SplitHostPort fails, assume it's just a hostname without port
		proxyHost = cfg.ProxyAddr
	}
	// Remove brackets from IPv6 addresses if present
	proxyHost = strings.Trim(proxyHost, "[]")

	// Create masque HTTP/3 client for UDP streams
	masqueH3Client, err := masquehttp3.NewClient(masquehttp3.ClientConfig{
		ProxyAddr:  cfg.ProxyAddr,
		AuthToken:  cfg.AuthToken,
		CertData:   cfg.CertData,
		Insecure:   cfg.IgnoreCert,
		Logger:     cfg.Logger,
	})
	if err != nil {
		return nil, fmt.Errorf("create masque HTTP/3 client: %w", err)
	}

	client := &MasqueClient{
		proxyAddr:      cfg.ProxyAddr,
		proxyHost:      proxyHost,
		authToken:      cfg.AuthToken,
		tlsConfig:      tlsConfig,
		logger:         cfg.Logger,
		masqueH3Client: masqueH3Client,
		h2Transport: &http2.Transport{
			TLSClientConfig: tlsConfig,
		},
	}

	return client, nil
}

// ConnectToProxy establishes HTTP/2 connection to the proxy server
func (c *MasqueClient) ConnectToProxy() error {
	// Test HTTP/2 connection with a simple roundtrip
	testURL := fmt.Sprintf("https://%s/", c.proxyAddr)
	req, err := http.NewRequest("GET", testURL, nil)
	if err != nil {
		return fmt.Errorf("create test request: %w", err)
	}

	if c.authToken != "" {
		req.Header.Set("Authorization", "Bearer "+c.authToken)
	}

	// Use HTTP/2 transport
	resp, err := c.h2Transport.RoundTrip(req)
	if err != nil {
		return fmt.Errorf("connect to proxy: %w", err)
	}
	defer resp.Body.Close()

	c.logger.Info("Connected to MASQUE proxy", "addr", c.proxyAddr, "protocol", resp.Proto)
	return nil
}

// CreateTCPStream creates a MASQUE TCP stream using HTTP/2 CONNECT
func (c *MasqueClient) CreateTCPStream(target string) (net.Conn, error) {
	// Parse target to ensure it's properly formatted
	host, port, err := net.SplitHostPort(target)
	if err != nil {
		return nil, fmt.Errorf("invalid target: %w", err)
	}

	// Reconstruct target (handles IPv6 properly)
	if strings.Contains(host, ":") {
		// IPv6 - ensure brackets
		target = fmt.Sprintf("[%s]:%s", strings.Trim(host, "[]"), port)
	} else {
		target = fmt.Sprintf("%s:%s", host, port)
	}

	// Create a new TLS connection for this stream
	tcpConn, err := net.Dial("tcp", c.proxyAddr)
	if err != nil {
		return nil, fmt.Errorf("dial proxy: %w", err)
	}

	// Clone TLS config and set ServerName for proper SNI
	tlsConfigWithSNI := c.tlsConfig.Clone()
	tlsConfigWithSNI.ServerName = c.proxyHost

	tlsConn := tls.Client(tcpConn, tlsConfigWithSNI)
	if err := tlsConn.Handshake(); err != nil {
		tcpConn.Close()
		return nil, fmt.Errorf("TLS handshake: %w", err)
	}

	// Verify HTTP/2 was negotiated
	state := tlsConn.ConnectionState()
	if state.NegotiatedProtocol != "h2" {
		tlsConn.Close()
		return nil, fmt.Errorf("HTTP/2 not negotiated: %s", state.NegotiatedProtocol)
	}

	// Create HTTP/2 client connection
	h2Conn, err := c.h2Transport.NewClientConn(tlsConn)
	if err != nil {
		tlsConn.Close()
		return nil, fmt.Errorf("create HTTP/2 connection: %w", err)
	}

	// Create pipe for request body (we won't use it for CONNECT, but it's required)
	pr, pw := io.Pipe()

	// Create HTTP CONNECT request
	// The URL doesn't matter for CONNECT, the Host header (which becomes :authority) is what matters
	req, err := http.NewRequest("CONNECT", "https://"+c.proxyAddr, pr)
	if err != nil {
		pw.Close()
		h2Conn.Close()
		return nil, fmt.Errorf("create request: %w", err)
	}

	// Set the target as the Host (becomes :authority pseudo-header in HTTP/2)
	req.Host = target
	req.Header.Set("User-Agent", "masque-vpn/1.0")
	if c.authToken != "" {
		req.Header.Set("Authorization", "Bearer "+c.authToken)
	}

	// Debug: Dump the HTTP/2 request details
	c.logger.Debug("=== HTTP/2 CONNECT Request ===")
	c.logger.Debug("Method", "value", req.Method)
	c.logger.Debug("URL", "value", req.URL.String())
	c.logger.Debug("Host", "value", req.Host)
	c.logger.Debug("Proto", "value", req.Proto)
	c.logger.Debug("=== Request Headers ===")
	for name, values := range req.Header {
		for _, value := range values {
			c.logger.Debug("Header", "name", name, "value", value)
		}
	}
	c.logger.Debug("=== End Request ===")

	// Perform the CONNECT request
	resp, err := h2Conn.RoundTrip(req)
	if err != nil {
		pw.Close()
		h2Conn.Close()
		c.logger.Error("HTTP/2 CONNECT RoundTrip error", "err", err)
		return nil, fmt.Errorf("CONNECT request: %w", err)
	}

	// Debug: Dump the response details
	c.logger.Debug("=== HTTP/2 Response ===")
	c.logger.Debug("Status", "code", resp.StatusCode, "text", resp.Status)
	c.logger.Debug("Proto", "value", resp.Proto)
	c.logger.Debug("=== Response Headers ===")
	for name, values := range resp.Header {
		for _, value := range values {
			c.logger.Debug("Header", "name", name, "value", value)
		}
	}
	c.logger.Debug("=== End Response ===")

	if resp.StatusCode != http.StatusOK {
		body, _ := io.ReadAll(resp.Body)
		resp.Body.Close()
		pw.Close()
		h2Conn.Close()
		c.logger.Error("HTTP/2 CONNECT failed", "status", resp.StatusCode, "body", string(body))
		return nil, fmt.Errorf("CONNECT failed: %d %s", resp.StatusCode, string(body))
	}

	c.logger.Debug("MASQUE TCP stream established", "target", target)

	// Return a connection that wraps the HTTP/2 stream
	return &h2StreamConn{
		readCloser: resp.Body,
		writer:     pw,
		h2Conn:     h2Conn,
	}, nil
}

// h2StreamConn wraps an HTTP/2 stream as a net.Conn
type h2StreamConn struct {
	readCloser io.ReadCloser
	writer     io.WriteCloser
	h2Conn     *http2.ClientConn
}

func (c *h2StreamConn) Read(b []byte) (n int, err error) {
	return c.readCloser.Read(b)
}

func (c *h2StreamConn) Write(b []byte) (n int, err error) {
	return c.writer.Write(b)
}

func (c *h2StreamConn) Close() error {
	// Close the stream components
	c.writer.Close()
	c.readCloser.Close()
	// Close the underlying HTTP/2 connection to prevent leaks
	// This also closes the TLS and TCP connections
	if c.h2Conn != nil {
		return c.h2Conn.Close()
	}
	return nil
}

func (c *h2StreamConn) LocalAddr() net.Addr {
	return &net.TCPAddr{IP: net.IPv4zero, Port: 0}
}

func (c *h2StreamConn) RemoteAddr() net.Addr {
	return &net.TCPAddr{IP: net.IPv4zero, Port: 0}
}

func (c *h2StreamConn) SetDeadline(t time.Time) error {
	return nil
}

func (c *h2StreamConn) SetReadDeadline(t time.Time) error {
	return nil
}

func (c *h2StreamConn) SetWriteDeadline(t time.Time) error {
	return nil
}

// UDPStreamWrapper wraps a DatagramStream and its dedicated HTTP/3 client
type UDPStreamWrapper struct {
	*masquehttp3.DatagramStream
	client *masquehttp3.Client
	logger *slog.Logger
}

// Close closes both the stream and its dedicated HTTP/3 client
func (w *UDPStreamWrapper) Close() error {
	w.logger.Debug("Closing UDP stream wrapper")
	// Close the stream first
	if w.DatagramStream != nil {
		if err := w.DatagramStream.Close(); err != nil {
			w.logger.Error("Error closing datagram stream", "err", err)
		}
	}
	// Then close the dedicated HTTP/3 client
	if w.client != nil {
		if err := w.client.Close(); err != nil {
			w.logger.Error("Error closing HTTP/3 client", "err", err)
			return err
		}
	}
	w.logger.Debug("UDP stream wrapper closed successfully")
	return nil
}

// CreateUDPStream creates a MASQUE UDP stream with a dedicated HTTP/3 connection
func (c *MasqueClient) CreateUDPStream(target string) (*UDPStreamWrapper, error) {
	// Parse target to ensure it's properly formatted
	host, port, err := net.SplitHostPort(target)
	if err != nil {
		return nil, fmt.Errorf("invalid target: %w", err)
	}

	// Reconstruct target (handles IPv6 properly)
	if strings.Contains(host, ":") {
		// IPv6 - ensure brackets
		target = fmt.Sprintf("[%s]:%s", strings.Trim(host, "[]"), port)
	} else {
		target = fmt.Sprintf("%s:%s", host, port)
	}

	// Create a NEW dedicated HTTP/3 client for this UDP stream
	// This prevents congestion on a single shared connection
	c.logger.Info("Creating dedicated HTTP/3 client for UDP stream", "target", target)
	dedicatedClient, err := masquehttp3.NewClient(masquehttp3.ClientConfig{
		ProxyAddr:  c.proxyAddr,
		AuthToken:  c.authToken,
		CertData:   nil, // Will use insecure for now
		Insecure:   true,
		Logger:     c.logger,
	})
	if err != nil {
		c.logger.Error("Failed to create dedicated HTTP/3 client", "err", err, "target", target)
		return nil, fmt.Errorf("create dedicated HTTP/3 client: %w", err)
	}

	// Create the UDP stream using the dedicated client
	c.logger.Debug("Creating UDP stream via dedicated client", "target", target)
	stream, err := dedicatedClient.CreateUDPStream(target)
	if err != nil {
		dedicatedClient.Close() // Clean up the client if stream creation fails
		c.logger.Error("Failed to create UDP stream", "err", err, "target", target)
		return nil, fmt.Errorf("create UDP stream: %w", err)
	}

	c.logger.Info("MASQUE UDP stream established with dedicated connection", "target", target)
	
	// Wrap both the stream and client so they're both closed together
	return &UDPStreamWrapper{
		DatagramStream: stream,
		client:         dedicatedClient,
		logger:         c.logger,
	}, nil
}

// Close closes the MASQUE client and all its connections
func (c *MasqueClient) Close() error {
	if c.masqueH3Client != nil {
		return c.masqueH3Client.Close()
	}
	return nil
}

// IsDisallowedPort checks if a port is in the disallowed list for MASQUE
func IsDisallowedPort(port uint16) bool {
	// Standard well-known ports that should not be proxied
	disallowedPorts := []uint16{
		1,    // tcpmux
		7,    // echo
		9,    // discard
		11,   // systat
		13,   // daytime
		15,   // netstat
		17,   // qotd
		19,   // chargen
		20,   // ftp-data
		21,   // ftp
		22,   // ssh
		23,   // telnet
		25,   // smtp
		37,   // time
		42,   // nameserver
		43,   // nicname
		53,   // dns
		77,   // rje
		79,   // finger
		87,   // link
		95,   // supdup
		101,  // hostname
		102,  // iso-tsap
		103,  // gppitnp
		104,  // acr-nema
		109,  // pop2
		110,  // pop3
		111,  // sunrpc
		113,  // auth
		115,  // sftp
		117,  // uucp-path
		119,  // nntp
		123,  // ntp
		135,  // msrpc
		139,  // netbios-ssn
		143,  // imap
		161,  // snmp
		179,  // bgp
		389,  // ldap
		427,  // svrloc
		465,  // submissions
		512,  // exec
		513,  // login
		514,  // shell
		515,  // printer
		526,  // tempo
		530,  // courier
		531,  // chat
		532,  // netnews
		540,  // uucp
		548,  // afp
		556,  // remotefs
		563,  // nntps
		587,  // submission
		601,  // syslog-conn
		636,  // ldaps
		993,  // imaps
		995,  // pop3s
		2049, // nfs
		3659, // apple-sasl
		4045, // lockd
		6000, // x11
		6665, // irc (alternate)
		6666, // irc (alternate)
		6667, // irc (default)
		6668, // irc (alternate)
		6669, // irc (alternate)
	}

	for _, p := range disallowedPorts {
		if port == p {
			return true
		}
	}
	return false
}
