package main

import (
	"context"
	"errors"
	"fmt"
	"log/slog"
	"net"
	"net/netip"

	"github.com/quic-go/quic-go"
	tun "github.com/sagernet/sing-tun"
	"github.com/sagernet/sing/common/buf"
	"github.com/sagernet/sing/common/bufio"
	M "github.com/sagernet/sing/common/metadata"
	N "github.com/sagernet/sing/common/network"
)

// masqueHandler receives the flows intercepted by sing-tun and proxies them
// through the MASQUE relay: TCP over HTTP/2 CONNECT, UDP over CONNECT-UDP,
// DNS over a shared CONNECT-UDP stream to the upstream resolver.
type masqueHandler struct {
	client *MasqueClient
	dns    *dnsForwarder
}

var _ tun.Handler = (*masqueHandler)(nil)

func targetAddr(destination M.Socksaddr) string {
	return netip.AddrPortFrom(destination.Addr.Unmap(), destination.Port).String()
}

func (h *masqueHandler) PrepareConnection(network string, source M.Socksaddr, destination M.Socksaddr) error {
	return nil
}

func (h *masqueHandler) NewConnectionEx(ctx context.Context, conn net.Conn, source M.Socksaddr, destination M.Socksaddr, onClose N.CloseHandlerFunc) {
	err := h.forwardTCP(ctx, conn, source, destination)
	if err != nil {
		logger.Debug("TCP connection closed", "target", targetAddr(destination), "err", err)
	}
	if onClose != nil {
		onClose(err)
	}
}

func (h *masqueHandler) forwardTCP(ctx context.Context, conn net.Conn, source M.Socksaddr, destination M.Socksaddr) error {
	target := targetAddr(destination)
	logger.Info("New TCP connection", "target", target, "source", source)

	masqueConn, err := h.client.CreateTCPStream(target)
	if err != nil {
		conn.Close()
		logger.Error("MASQUE create failed", "err", err, "target", target)
		return err
	}

	// CopyConn closes both ends when done.
	return bufio.CopyConn(ctx, conn, masqueConn)
}

func (h *masqueHandler) NewPacketConnectionEx(ctx context.Context, conn N.PacketConn, source M.Socksaddr, destination M.Socksaddr, onClose N.CloseHandlerFunc) {
	err := h.forwardUDP(conn, source)
	if err != nil {
		logger.Debug("UDP session closed", "source", source, "err", err)
	}
	if onClose != nil {
		onClose(err)
	}
}

// forwardUDP serves one sing-tun UDP session. Sessions are keyed by source
// address only, so packets can go to several destinations; each destination
// gets its own CONNECT-UDP stream, since those are bound to a single target.
// sing-tun closes conn once the session is idle for udpTimeout.
func (h *masqueHandler) forwardUDP(conn N.PacketConn, source M.Socksaddr) error {
	defer conn.Close()

	streams := make(map[M.Socksaddr]*udpStream)
	defer func() {
		for _, stream := range streams {
			stream.Close()
		}
	}()

	for {
		buffer := buf.NewPacket()
		destination, err := conn.ReadPacket(buffer)
		if err != nil {
			buffer.Release()
			return err
		}

		if destination.Port == 53 {
			query := append([]byte(nil), buffer.Bytes()...)
			buffer.Release()
			go h.dns.Handle(query, destination, conn)
			continue
		}

		stream := streams[destination]
		if stream == nil {
			target := targetAddr(destination)
			logger.Info("New UDP flow", "target", target, "source", source)
			stream, err = h.client.CreateUDPStream(target)
			if err != nil {
				buffer.Release()
				logger.Error("MASQUE UDP create failed", "err", err, "target", target)
				continue
			}
			streams[destination] = stream
			go downloadUDP(conn, stream, destination)
		}

		_, err = stream.Write(buffer.Bytes())
		buffer.Release()
		// Datagrams that don't fit in a QUIC packet are dropped, like on a
		// link with a smaller MTU.
		var tooLarge *quic.DatagramTooLargeError
		if err != nil && !errors.As(err, &tooLarge) {
			logger.Debug("UDP flow closed", "target", targetAddr(destination), "err", err)
			stream.Close()
			delete(streams, destination)
		}
	}
}

func downloadUDP(conn N.PacketConn, stream *udpStream, destination M.Socksaddr) {
	for {
		buffer := buf.NewPacket()
		n, err := stream.Read(buffer.FreeBytes())
		if err != nil {
			buffer.Release()
			return
		}
		buffer.Truncate(n)
		// The system stack copies the payload and leaves buffer to us.
		err = conn.WritePacket(buffer, destination)
		buffer.Release()
		if err != nil {
			return
		}
	}
}

// singLogger adapts slog to the logger interface sing-tun expects.
type singLogger struct {
	logger *slog.Logger
}

func newSingLogger(logger *slog.Logger) *singLogger {
	return &singLogger{logger: logger.With("component", "sing-tun")}
}

func (l *singLogger) Trace(args ...any) { l.logger.Debug(fmt.Sprint(args...)) }
func (l *singLogger) Debug(args ...any) { l.logger.Debug(fmt.Sprint(args...)) }
func (l *singLogger) Info(args ...any)  { l.logger.Info(fmt.Sprint(args...)) }
func (l *singLogger) Warn(args ...any)  { l.logger.Warn(fmt.Sprint(args...)) }
func (l *singLogger) Error(args ...any) { l.logger.Error(fmt.Sprint(args...)) }
func (l *singLogger) Fatal(args ...any) { l.logger.Error(fmt.Sprint(args...)) }
func (l *singLogger) Panic(args ...any) { l.logger.Error(fmt.Sprint(args...)) }
