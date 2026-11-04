// MASQUE client: TCP over HTTP/2 CONNECT, UDP over HTTP/3 CONNECT-UDP
// (RFC 9298).
package main

import (
	"context"
	"crypto/tls"
	"crypto/x509"
	"fmt"
	"io"
	"net"
	"net/http"
	"net/url"
	"sync"
	"sync/atomic"
	"time"

	"github.com/quic-go/quic-go"
	"github.com/quic-go/quic-go/http3"
	"github.com/quic-go/quic-go/quicvarint"
	"github.com/yosida95/uritemplate/v3"
	"golang.org/x/net/http2"
)

const quicDialTimeout = 10 * time.Second

type MasqueClient struct {
	proxyAddr string

	// The token is read for every new stream and can be rotated at any time;
	// open streams keep the token they were created with.
	authToken atomic.Pointer[string]

	// Pools connections to the proxy and multiplexes CONNECT streams on them.
	h2 *http2.Transport

	h3TLS       *tls.Config
	udpTemplate *uritemplate.Template

	// All UDP flows share one QUIC connection; it is redialed once it dies.
	h3Mu   sync.Mutex
	h3Conn *quic.Conn
	h3     *http3.ClientConn
}

func NewClient(proxyAddr, token string, certData []byte, insecure bool) (*MasqueClient, error) {
	tlsConfig := &tls.Config{InsecureSkipVerify: insecure}
	if len(certData) > 0 {
		certPool := x509.NewCertPool()
		if !certPool.AppendCertsFromPEM(certData) {
			return nil, fmt.Errorf("no PEM certificate found")
		}
		tlsConfig.RootCAs = certPool
	}
	h3TLS := tlsConfig.Clone()
	h3TLS.NextProtos = []string{http3.NextProtoH3}

	// Default RFC 9298 template.
	udpTemplate, err := uritemplate.New("https://" + proxyAddr + "/.well-known/masque/udp/{target_host}/{target_port}/")
	if err != nil {
		return nil, err
	}

	c := &MasqueClient{
		proxyAddr:   proxyAddr,
		h2:          &http2.Transport{TLSClientConfig: tlsConfig},
		h3TLS:       h3TLS,
		udpTemplate: udpTemplate,
	}
	c.SetAuthToken(token)
	return c, nil
}

func (c *MasqueClient) AuthToken() string {
	return *c.authToken.Load()
}

func (c *MasqueClient) SetAuthToken(token string) {
	c.authToken.Store(&token)
}

// CreateTCPStream opens an HTTP/2 CONNECT stream to target ("ip:port").
func (c *MasqueClient) CreateTCPStream(target string) (net.Conn, error) {
	pr, pw := io.Pipe()
	// The URL picks the proxy connection; Host becomes :authority (the target).
	req, err := http.NewRequest(http.MethodConnect, "https://"+c.proxyAddr, pr)
	if err != nil {
		return nil, err
	}
	req.Host = target
	req.Header.Set("Proxy-Authorization", "Bearer "+c.AuthToken())

	resp, err := c.h2.RoundTrip(req)
	if err != nil {
		pw.Close()
		return nil, fmt.Errorf("CONNECT %s: %w", target, err)
	}
	if resp.StatusCode != http.StatusOK {
		resp.Body.Close()
		pw.Close()
		return nil, fmt.Errorf("CONNECT %s: %s", target, resp.Status)
	}
	return &h2StreamConn{ReadCloser: resp.Body, writer: pw}, nil
}

// h2StreamConn exposes a CONNECT stream as the net.Conn sing's copy helpers
// expect. Addresses and deadlines are not meaningful for it.
type h2StreamConn struct {
	io.ReadCloser
	writer *io.PipeWriter
}

func (c *h2StreamConn) Write(b []byte) (int, error) { return c.writer.Write(b) }

// CloseWrite ends the request body (END_STREAM) so a half-close from the
// local side is propagated to the target instead of tearing down the stream.
func (c *h2StreamConn) CloseWrite() error { return c.writer.Close() }

func (c *h2StreamConn) Close() error {
	c.writer.Close()
	return c.ReadCloser.Close()
}

func (c *h2StreamConn) LocalAddr() net.Addr                { return &net.TCPAddr{} }
func (c *h2StreamConn) RemoteAddr() net.Addr               { return &net.TCPAddr{} }
func (c *h2StreamConn) SetDeadline(t time.Time) error      { return nil }
func (c *h2StreamConn) SetReadDeadline(t time.Time) error  { return nil }
func (c *h2StreamConn) SetWriteDeadline(t time.Time) error { return nil }

// CreateUDPStream opens a CONNECT-UDP stream to target ("ip:port").
func (c *MasqueClient) CreateUDPStream(target string) (*udpStream, error) {
	h3, err := c.h3ClientConn()
	if err != nil {
		return nil, err
	}
	host, port, err := net.SplitHostPort(target)
	if err != nil {
		return nil, err
	}
	expanded, err := c.udpTemplate.Expand(uritemplate.Values{
		"target_host": uritemplate.String(host),
		"target_port": uritemplate.String(port),
	})
	if err != nil {
		return nil, err
	}
	u, err := url.Parse(expanded)
	if err != nil {
		return nil, err
	}

	ctx, cancel := context.WithTimeout(context.Background(), quicDialTimeout)
	defer cancel()
	select {
	case <-h3.ReceivedSettings():
	case <-h3.Context().Done():
		return nil, context.Cause(h3.Context())
	case <-ctx.Done():
		return nil, ctx.Err()
	}
	if settings := h3.Settings(); !settings.EnableExtendedConnect || !settings.EnableDatagrams {
		return nil, fmt.Errorf("proxy doesn't support Extended CONNECT with datagrams")
	}

	str, err := h3.OpenRequestStream(ctx)
	if err != nil {
		return nil, err
	}
	err = str.SendRequestHeader(&http.Request{
		Method: http.MethodConnect,
		Proto:  "connect-udp",
		Host:   u.Host,
		URL:    u,
		Header: http.Header{
			http3.CapsuleProtocolHeader: {"?1"},
			"Proxy-Authorization":       {"Bearer " + c.AuthToken()},
		},
	})
	if err == nil {
		str.SetReadDeadline(time.Now().Add(quicDialTimeout))
		var resp *http.Response
		resp, err = str.ReadResponse()
		if err == nil && (resp.StatusCode < 200 || resp.StatusCode > 299) {
			err = fmt.Errorf("%s", resp.Status)
		}
		str.SetReadDeadline(time.Time{})
	}
	if err != nil {
		str.CancelRead(quic.StreamErrorCode(http3.ErrCodeNoError))
		str.CancelWrite(quic.StreamErrorCode(http3.ErrCodeNoError))
		return nil, fmt.Errorf("CONNECT-UDP %s: %w", target, err)
	}

	s := &udpStream{str: str}
	s.ctx, s.cancel = context.WithCancel(context.Background())
	// No capsule types are used, so the stream itself only signals the end
	// of the flow.
	go func() {
		io.Copy(io.Discard, str)
		s.cancel()
	}()
	return s, nil
}

// udpStream carries UDP payloads as HTTP datagrams with context ID 0.
type udpStream struct {
	str    *http3.RequestStream
	ctx    context.Context
	cancel context.CancelFunc
}

func (s *udpStream) Read(b []byte) (int, error) {
	for {
		data, err := s.str.ReceiveDatagram(s.ctx)
		if err != nil {
			return 0, err
		}
		contextID, n, err := quicvarint.Parse(data)
		if err != nil {
			return 0, fmt.Errorf("malformed datagram: %w", err)
		}
		if contextID == 0 {
			return copy(b, data[n:]), nil
		}
	}
}

func (s *udpStream) Write(b []byte) (int, error) {
	if err := s.ctx.Err(); err != nil {
		return 0, err
	}
	return len(b), s.str.SendDatagram(append([]byte{0}, b...))
}

func (s *udpStream) Close() error {
	s.cancel()
	s.str.CancelRead(quic.StreamErrorCode(http3.ErrCodeNoError))
	return s.str.Close()
}

func (c *MasqueClient) h3ClientConn() (*http3.ClientConn, error) {
	c.h3Mu.Lock()
	defer c.h3Mu.Unlock()
	if c.h3Conn != nil && c.h3Conn.Context().Err() == nil {
		return c.h3, nil
	}

	ctx, cancel := context.WithTimeout(context.Background(), quicDialTimeout)
	defer cancel()
	conn, err := quic.DialAddr(ctx, c.proxyAddr, c.h3TLS, &quic.Config{
		EnableDatagrams: true,
		// Leaves room for tunnelled QUIC, which needs 1200-byte packets.
		InitialPacketSize: 1350,
		KeepAlivePeriod:   25 * time.Second,
	})
	if err != nil {
		return nil, fmt.Errorf("dial HTTP/3 proxy: %w", err)
	}
	h3 := (&http3.Transport{EnableDatagrams: true}).NewClientConn(conn)
	c.h3Conn, c.h3 = conn, h3
	return h3, nil
}

func (c *MasqueClient) Close() {
	c.h2.CloseIdleConnections()
	c.h3Mu.Lock()
	defer c.h3Mu.Unlock()
	if c.h3Conn != nil {
		c.h3Conn.CloseWithError(0, "")
	}
}
