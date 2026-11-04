package main

import (
	"encoding/binary"
	"errors"
	"net/netip"
	"strings"
	"sync"
	"time"

	"github.com/sagernet/sing/common/buf"
	M "github.com/sagernet/sing/common/metadata"
	N "github.com/sagernet/sing/common/network"
	"golang.org/x/net/dns/dnsmessage"
)

const (
	dnsQueryTimeout = 5 * time.Second
	dnsRelayTTL     = 60
)

// dnsForwarder handles every UDP/53 packet hijacked by sing-tun.
//
// Queries for the relay hostname are answered locally from the addresses
// resolved at startup. The daemon itself re-resolves the relay every time it
// opens a stream, and once the routes are up that lookup is hijacked here too;
// forwarding it through MASQUE would need a stream, which needs the lookup.
//
// Everything else goes to the upstream resolver over a single shared
// CONNECT-UDP stream, rewriting DNS IDs so queries from different clients
// can't collide.
type dnsForwarder struct {
	client    *MasqueClient
	upstream  netip.AddrPort
	relayHost string // lowercased, no trailing dot; empty when relay is an IP
	relayIPs  []netip.Addr

	streamMu sync.Mutex
	stream   *udpStream

	pendingMu sync.Mutex
	pending   map[uint16]*dnsQuery
	nextID    uint16
	closed    bool
}

type dnsQuery struct {
	id          uint16
	destination M.Socksaddr
	writer      N.PacketWriter
	timer       *time.Timer
}

func newDNSForwarder(client *MasqueClient, upstream netip.AddrPort, relay string, relayIPs []netip.Addr) *dnsForwarder {
	d := &dnsForwarder{
		client:   client,
		upstream: upstream,
		relayIPs: relayIPs,
		pending:  make(map[uint16]*dnsQuery),
	}
	if _, err := netip.ParseAddr(relay); err != nil {
		d.relayHost = strings.ToLower(strings.TrimSuffix(relay, "."))
	}
	return d
}

func (d *dnsForwarder) Handle(query []byte, destination M.Socksaddr, writer N.PacketWriter) {
	if response, ok := d.answerLocally(query); ok {
		logger.Debug("Answered relay DNS query locally", "relay", d.relayHost)
		writeDNSResponse(writer, response, destination)
		return
	}
	if err := d.forward(query, destination, writer); err != nil {
		logger.Error("DNS forward failed", "err", err, "upstream", d.upstream)
	}
}

func writeDNSResponse(writer N.PacketWriter, response []byte, destination M.Socksaddr) {
	buffer := buf.As(response)
	if err := writer.WritePacket(buffer, destination); err != nil {
		logger.Debug("DNS response write failed", "err", err)
	}
}

// answerLocally builds a response for A/AAAA queries for the relay hostname.
func (d *dnsForwarder) answerLocally(query []byte) ([]byte, bool) {
	if d.relayHost == "" {
		return nil, false
	}

	var parser dnsmessage.Parser
	header, err := parser.Start(query)
	if err != nil || header.Response {
		return nil, false
	}
	question, err := parser.Question()
	if err != nil {
		return nil, false
	}
	name := strings.ToLower(strings.TrimSuffix(question.Name.String(), "."))
	if name != d.relayHost {
		return nil, false
	}

	builder := dnsmessage.NewBuilder(nil, dnsmessage.Header{
		ID:                 header.ID,
		Response:           true,
		Authoritative:      true,
		RecursionDesired:   header.RecursionDesired,
		RecursionAvailable: true,
	})
	builder.EnableCompression()
	if err := builder.StartQuestions(); err != nil {
		return nil, false
	}
	if err := builder.Question(question); err != nil {
		return nil, false
	}
	if err := builder.StartAnswers(); err != nil {
		return nil, false
	}
	resourceHeader := dnsmessage.ResourceHeader{
		Name:  question.Name,
		Class: dnsmessage.ClassINET,
		TTL:   dnsRelayTTL,
	}
	// Other query types (and the family we have no address for) get an
	// empty NOERROR answer.
	for _, addr := range d.relayIPs {
		switch {
		case question.Type == dnsmessage.TypeA && addr.Is4():
			err = builder.AResource(resourceHeader, dnsmessage.AResource{A: addr.As4()})
		case question.Type == dnsmessage.TypeAAAA && addr.Is6():
			err = builder.AAAAResource(resourceHeader, dnsmessage.AAAAResource{AAAA: addr.As16()})
		}
		if err != nil {
			return nil, false
		}
	}
	response, err := builder.Finish()
	if err != nil {
		return nil, false
	}
	return response, true
}

func (d *dnsForwarder) forward(query []byte, destination M.Socksaddr, writer N.PacketWriter) error {
	if len(query) < 12 {
		return errors.New("short DNS query")
	}

	stream, err := d.getStream()
	if err != nil {
		return err
	}

	q := &dnsQuery{
		id:          binary.BigEndian.Uint16(query),
		destination: destination,
		writer:      writer,
	}
	id, err := d.addPending(q)
	if err != nil {
		return err
	}

	rewritten := append([]byte(nil), query...)
	binary.BigEndian.PutUint16(rewritten, id)
	if _, err := stream.Write(rewritten); err != nil {
		d.takePending(id, q)
		d.dropStream(stream)
		return err
	}
	return nil
}

func (d *dnsForwarder) addPending(q *dnsQuery) (uint16, error) {
	d.pendingMu.Lock()
	defer d.pendingMu.Unlock()
	if d.closed {
		return 0, errors.New("DNS forwarder closed")
	}
	if len(d.pending) >= 1<<16 {
		return 0, errors.New("too many pending DNS queries")
	}
	for {
		d.nextID++
		if _, used := d.pending[d.nextID]; !used {
			break
		}
	}
	id := d.nextID
	d.pending[id] = q
	q.timer = time.AfterFunc(dnsQueryTimeout, func() {
		d.takePending(id, q)
	})
	return id, nil
}

// takePending removes the query registered under id. When expected is set it
// is only removed if it is still that query (it may have been reused).
func (d *dnsForwarder) takePending(id uint16, expected *dnsQuery) *dnsQuery {
	d.pendingMu.Lock()
	defer d.pendingMu.Unlock()
	q := d.pending[id]
	if q == nil || (expected != nil && q != expected) {
		return nil
	}
	delete(d.pending, id)
	q.timer.Stop()
	return q
}

func (d *dnsForwarder) getStream() (*udpStream, error) {
	d.streamMu.Lock()
	defer d.streamMu.Unlock()
	if d.stream != nil {
		return d.stream, nil
	}
	stream, err := d.client.CreateUDPStream(d.upstream.String())
	if err != nil {
		return nil, err
	}
	d.stream = stream
	go d.readLoop(stream)
	return stream, nil
}

func (d *dnsForwarder) dropStream(stream *udpStream) {
	d.streamMu.Lock()
	if d.stream == stream {
		d.stream = nil
	}
	d.streamMu.Unlock()
	stream.Close()
}

func (d *dnsForwarder) readLoop(stream *udpStream) {
	defer d.dropStream(stream)
	packet := make([]byte, 65535)
	for {
		n, err := stream.Read(packet)
		if err != nil {
			logger.Debug("DNS upstream stream closed", "err", err)
			return
		}
		if n < 12 {
			continue
		}
		q := d.takePending(binary.BigEndian.Uint16(packet), nil)
		if q == nil {
			continue
		}
		response := append([]byte(nil), packet[:n]...)
		binary.BigEndian.PutUint16(response, q.id)
		writeDNSResponse(q.writer, response, q.destination)
	}
}

func (d *dnsForwarder) Close() {
	d.pendingMu.Lock()
	d.closed = true
	for id, q := range d.pending {
		q.timer.Stop()
		delete(d.pending, id)
	}
	d.pendingMu.Unlock()

	d.streamMu.Lock()
	stream := d.stream
	d.stream = nil
	d.streamMu.Unlock()
	if stream != nil {
		stream.Close()
	}
}
