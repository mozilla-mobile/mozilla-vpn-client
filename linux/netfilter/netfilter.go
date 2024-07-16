/* SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2019 Edge Security LLC. All Rights Reserved.
 */

package main

// #include <stdlib.h>
// #include <sys/types.h>
// static void callLogger(void *func, int level, const char *msg)
// {
//   ((void(*)(int, const char *))func)(level, msg);
// }
import "C"

import (
	"bytes"
	"errors"
	"log"
	"net"
	"unsafe"

	"C"

	"github.com/google/nftables"
	"github.com/google/nftables/alignedbuff"
	"github.com/google/nftables/binaryutil"
	"github.com/google/nftables/expr"
	"github.com/google/nftables/xt"
	linux "golang.org/x/sys/unix"
)
import (
	"fmt"
)

type CLogger struct {
	level    C.int
	function unsafe.Pointer
}

func (l *CLogger) Write(p []byte) (int, error) {
	if uintptr(l.function) == 0 {
		return 0, errors.New("No logger initialized")
	}
	message := C.CString(string(p))
	C.callLogger(l.function, l.level, message)
	C.free(unsafe.Pointer(message))
	return len(p), nil
}

//export NetfilterSetLogger
func NetfilterSetLogger(loggerFn uintptr) {
	log.SetOutput(&CLogger{
		level:    0,
		function: unsafe.Pointer(loggerFn),
	})
	log.SetPrefix("")
	log.SetFlags(0)
}

type nftCtx struct {
	table       *nftables.Table
	cgroup_mark *nftables.Chain
	cgroup_nat  *nftables.Chain
	conntrack   *nftables.Chain
	preroute    *nftables.Chain
	input       *nftables.Chain
	output      *nftables.Chain
	addrset     *nftables.Set
	fwmark      uint32
	conn        nftables.Conn
}

var mozvpn_ctx = nftCtx{}

func (ctx *nftCtx) nftCommit() int32 {
	if err := ctx.conn.Flush(); err != nil {
		log.Println("Netfiler commit failed:", err)
		return -1
	}
	log.Println("Netfiler commit succeeded")
	return 0
}

func nftIfname(n string) []byte {
	b := make([]byte, 16)
	copy(b, []byte(n+"\x00"))
	return b
}

// A Conntrack mark used to identify traffic excluded from the VPN.
// The value is not important, so long as it's constant, unique,
// and non-zero.
const ctmark_excluded = 0x9e4c

func (ctx *nftCtx) nftApplyFwMark() {
	// Apply a conntrack mark to excluded outbound packets so that we can
	// identify the corresponding inbound packet flows.
	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.conntrack,
		Exprs: []expr.Any{
			// Match packets with a fwmark set.
			&expr.Meta{
				Key:      expr.MetaKeyMARK,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			// Set the conntrack mark.
			&expr.Immediate{
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctmark_excluded),
			},
			&expr.Ct{
				Key:            expr.CtKeyMARK,
				Register:       1,
				SourceRegister: true,
			},
		},
	})

	// Inbound packets from wireguard servers should be marked for RPF.
	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.preroute,
		Exprs: []expr.Any{
			// For now, we only support IPv4 in this check.
			// TODO: Support IPv6.
			&expr.Meta{
				Key:      expr.MetaKeyNFPROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.NFPROTO_IPV4},
			},
			// Match UDP packets
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			// Lookup the IPv4 source address.
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseNetworkHeader,
				Offset:       uint32(12),
				Len:          uint32(4),
			},
			&expr.Lookup{
				SourceRegister: 1,
				SetName:        ctx.addrset.Name,
				SetID:          ctx.addrset.ID,
			},
			// Set the firewall mark.
			&expr.Immediate{
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			&expr.Meta{
				Key:            expr.MetaKeyMARK,
				Register:       1,
				SourceRegister: true,
			},
		},
	})
}

func (ctx *nftCtx) nftRestrictTraffic(ifname string) {
	log.Println("Restricting network traffic", ifname)

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.input,
		Exprs: []expr.Any{
			// Accept packet from loopback interfaces
			&expr.Meta{
				Key:      expr.MetaKeyIIFTYPE,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint16(linux.ARPHRD_LOOPBACK),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.input,
		Exprs: []expr.Any{
			// Accept all packets from the VPN interface
			&expr.Meta{
				Key:      expr.MetaKeyIIFNAME,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     nftIfname(ifname),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.input,
		Exprs: []expr.Any{
			// Accept all packets from the VPN server
			&expr.Meta{
				Key:      expr.MetaKeyPROTOCOL,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(linux.ETH_P_IP),
			},
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseNetworkHeader,
				Offset:       uint32(12), // saddr
				Len:          uint32(4),
			},
			&expr.Lookup{
				SourceRegister: 1,
				SetName:        ctx.addrset.Name,
				SetID:          ctx.addrset.ID,
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.input,
		Exprs: []expr.Any{
			// Accept packets if they originated from an excluded connection.
			// TODO: Any other conntrack state we should check?
			&expr.Ct{
				Key:      expr.CtKeyMARK,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctmark_excluded),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.input,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyMARK,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	// Output

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.output,
		Exprs: []expr.Any{
			// Accept packet to loopback interfaces
			&expr.Meta{
				Key:      expr.MetaKeyOIFTYPE,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint16(linux.ARPHRD_LOOPBACK),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.output,
		Exprs: []expr.Any{
			// Accept all packets to the VPN interface
			&expr.Meta{
				Key:      expr.MetaKeyOIFNAME,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     nftIfname(ifname),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.output,
		Exprs: []expr.Any{
			// Accept all packets to the VPN server
			&expr.Meta{
				Key:      expr.MetaKeyPROTOCOL,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(linux.ETH_P_IP),
			},
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseNetworkHeader,
				Offset:       uint32(16), // daddr
				Len:          uint32(4),
			},
			&expr.Lookup{
				SourceRegister: 1,
				SetName:        ctx.addrset.Name,
				SetID:          ctx.addrset.ID,
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.output,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyMARK,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})
}

func nftXtCgroupMatch(cgroup string) expr.Match {
	// struct xt_cgroup_info_v2 only supports a maximum path of 512 bytes
	if len(cgroup) >= 512 {
		cgroup = cgroup[:511]
	}

	// Build struct xt_cgroup_info_v2 to match the cgroup path.
	xtcgroup := alignedbuff.New()
	xtcgroup.PutUint8(1)                                   // has_path
	xtcgroup.PutUint8(0)                                   // has_classid
	xtcgroup.PutUint8(0)                                   // invert_path
	xtcgroup.PutUint8(0)                                   // invert_classid
	xtcgroup.PutBytesAligned32([]byte(cgroup+"\x00"), 512) // Cgroup path.
	xtcgroup.PutUint64(0)                                  // kernel padding
	var xtinfo = xt.Unknown(xtcgroup.Data())

	return expr.Match{
		Name: "cgroup",
		Rev:  2,
		Info: &xtinfo,
	}
}

func (ctx *nftCtx) nftMarkCgroup2xt(cgroup string) {
	xtmatch := nftXtCgroupMatch(cgroup)

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.cgroup_mark,
		Exprs: []expr.Any{
			// Match the cgroup v2 path for originated packets.
			&xtmatch,
			// Match packets that have not already been marked
			&expr.Meta{
				Key:      expr.MetaKeyMARK,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(0),
			},
			// Do not match packets sent to localhost interfaces
			&expr.Meta{
				Key:      expr.MetaKeyOIFTYPE,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpNeq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint16(linux.ARPHRD_LOOPBACK),
			},
			// Set the firewall mark to route this packet outside of the VPN.
			&expr.Immediate{
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			&expr.Meta{
				Key:            expr.MetaKeyMARK,
				Register:       1,
				SourceRegister: true,
			},
		},
	})

	// Masquerade outgoing packets from split tunnelling to trigger rerouting.
	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.cgroup_nat,
		Exprs: []expr.Any{
			&xtmatch,
			// Match marked packets
			&expr.Meta{
				Key:      expr.MetaKeyMARK,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			// Masquerade
			&expr.Masq{},
		},
	})
}

func (ctx *nftCtx) nftDelCgroup2xt(rules []*nftables.Rule, xtmatch *expr.Match) {
	cgdata, _ := xt.Marshal(0, xtmatch.Rev, xtmatch.Info)

	for _, r := range rules {
		rr := r.Exprs[0].(*expr.Match)
		if rr.Name != xtmatch.Name || rr.Rev != xtmatch.Rev {
			continue
		}
		rrdata, err := xt.Marshal(0, rr.Rev, rr.Info)
		if err != nil {
			continue
		}
		if bytes.Compare(rrdata, cgdata) == 0 {
			log.Println("Deleting", r.Chain.Name, "rule handle", r.Handle)
			ctx.conn.DelRule(r)
		}
	}
}

func (ctx *nftCtx) nftMarkCgroup1netcls(cgroup uint32) {
	// Match packets originating from the cgroup/net_cls
	var loadcgroup = expr.Meta{
		Key:      expr.MetaKeyCGROUP,
		Register: 1,
	}
	var matchcgroup = expr.Cmp{
		Op:       expr.CmpOpEq,
		Register: 1,
		Data:     binaryutil.NativeEndian.PutUint32(cgroup),
	}

	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.cgroup_mark,
		Exprs: []expr.Any{
			&loadcgroup,
			&matchcgroup,
			// Do not match packets sent to localhost interfaces
			&expr.Meta{
				Key:      expr.MetaKeyOIFTYPE,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpNeq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint16(linux.ARPHRD_LOOPBACK),
			},
			// Set the firewall mark
			&expr.Immediate{
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			&expr.Meta{
				Key:            expr.MetaKeyMARK,
				Register:       1,
				SourceRegister: true,
			},
		},
	})

	// Masquerade outgoing packets from split tunnelling to trigger rerouting.
	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.cgroup_nat,
		Exprs: []expr.Any{
			&loadcgroup,
			&matchcgroup,
			// Match marked packets
			&expr.Meta{
				Key:      expr.MetaKeyMARK,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(ctx.fwmark),
			},
			// Masquerade
			&expr.Masq{},
		},
	})
}

func (ctx *nftCtx) nftBlockCgroup(cgroup uint32) {
	ctx.conn.AddRule(&nftables.Rule{
		Table: ctx.table,
		Chain: ctx.cgroup_mark,
		Exprs: []expr.Any{
			// Match packets from the cgroup
			&expr.Meta{
				Key:      expr.MetaKeyCGROUP,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(cgroup),
			},
			// Do not match packets sent to localhost interfaces
			&expr.Meta{
				Key:      expr.MetaKeyOIFTYPE,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpNeq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint16(linux.ARPHRD_LOOPBACK),
			},
			// Drop the packets
			&expr.Verdict{
				Kind: expr.VerdictDrop,
			},
		},
	})
}

//export NetfilterCreateTables
func NetfilterCreateTables() int32 {
	mozvpn_ctx.table = mozvpn_ctx.conn.AddTable(&nftables.Table{
		Family: nftables.TableFamilyINet,
		Name:   "mozvpn-inet",
	})
	mozvpn_ctx.cgroup_mark = mozvpn_ctx.conn.AddChain(&nftables.Chain{
		Name:     "mozvpn-cgroup-mark",
		Table:    mozvpn_ctx.table,
		Type:     nftables.ChainTypeRoute,
		Hooknum:  nftables.ChainHookOutput,
		Priority: nftables.ChainPriorityRaw,
	})
	mozvpn_ctx.cgroup_nat = mozvpn_ctx.conn.AddChain(&nftables.Chain{
		Name:     "mozvpn-cgroup-nat",
		Table:    mozvpn_ctx.table,
		Type:     nftables.ChainTypeNAT,
		Hooknum:  nftables.ChainHookPostrouting,
		Priority: nftables.ChainPriorityNATSource,
	})
	mozvpn_ctx.conntrack = mozvpn_ctx.conn.AddChain(&nftables.Chain{
		Name:     "mozvpn-conntrack",
		Table:    mozvpn_ctx.table,
		Type:     nftables.ChainTypeRoute,
		Hooknum:  nftables.ChainHookOutput,
		Priority: nftables.ChainPriorityConntrack + 1,
	})
	mozvpn_ctx.preroute = mozvpn_ctx.conn.AddChain(&nftables.Chain{
		Name:     "mozvpn-preroute",
		Table:    mozvpn_ctx.table,
		Type:     nftables.ChainTypeFilter,
		Hooknum:  nftables.ChainHookPrerouting,
		Priority: nftables.ChainPriorityRaw,
	})

	dropPolicy := nftables.ChainPolicyDrop
	mozvpn_ctx.input = mozvpn_ctx.conn.AddChain(&nftables.Chain{
		Name:     "mozvpn-input",
		Table:    mozvpn_ctx.table,
		Type:     nftables.ChainTypeFilter,
		Hooknum:  nftables.ChainHookInput,
		Priority: nftables.ChainPriorityFilter,
		Policy:   &dropPolicy,
	})
	mozvpn_ctx.output = mozvpn_ctx.conn.AddChain(&nftables.Chain{
		Name:     "mozvpn-output",
		Table:    mozvpn_ctx.table,
		Type:     nftables.ChainTypeFilter,
		Hooknum:  nftables.ChainHookOutput,
		Priority: nftables.ChainPriorityFilter,
		Policy:   &dropPolicy,
	})

	mozvpn_ctx.addrset = &nftables.Set{
		Table:   mozvpn_ctx.table,
		Name:    "mozvpn-addrset",
		KeyType: nftables.TypeIPAddr,
	}
	mozvpn_ctx.conn.AddSet(mozvpn_ctx.addrset, nil)

	log.Println("Creating netfilter tables")
	return mozvpn_ctx.nftCommit()
}

//export NetfilterRemoveTables
func NetfilterRemoveTables() int32 {
	tables, err := mozvpn_ctx.conn.ListTables()
	if err != nil {
		fmt.Println("Error listing tables:", err)
		return -1
	}

	for _, table := range tables {
		if table.Name == "mozvpn-inet" {
			log.Println("Removing netfilter table")
			mozvpn_ctx.conn.DelTable(table)
		}
	}

	return mozvpn_ctx.nftCommit()
}

//export NetfilterApplyFwMark
func NetfilterApplyFwMark(fwmark uint32) int32 {
	mozvpn_ctx.fwmark = fwmark
	if fwmark == 0 {
		log.Panic("Invalid fwmark")
		return -1
	}

	mozvpn_ctx.nftApplyFwMark()

	log.Println("Applying firewall mark")
	return mozvpn_ctx.nftCommit()
}

//export NetfilterRestrictTraffic
func NetfilterRestrictTraffic(ifname string) int32 {
	mozvpn_ctx.nftRestrictTraffic(ifname)

	log.Println("Restricting traffic for", ifname)
	return mozvpn_ctx.nftCommit()
}

const (
	SADDR = iota
	DADDR
)

func buildIpNetExpr(ipnet *net.IPNet, addrtype int) []expr.Any {
	ipsize := len(ipnet.IP)

	var offset uint32
	var iptype []byte
	var ip []byte
	var xormask = make([]byte, ipsize)
	if ipnet.IP.To4() != nil {
		iptype = []byte{linux.NFPROTO_IPV4}
		ip = ipnet.IP.To4()

		if addrtype == SADDR {
			offset = 12
		} else {
			offset = 16
		}
	} else {
		iptype = []byte{linux.NFPROTO_IPV6}
		ip = ipnet.IP.To16()

		if addrtype == SADDR {
			offset = 8
		} else {
			offset = 24
		}
	}

	return []expr.Any{
		&expr.Meta{
			Key:      expr.MetaKeyNFPROTO,
			Register: 1,
		},
		&expr.Cmp{
			Op:       expr.CmpOpEq,
			Register: 1,
			Data:     iptype,
		},
		&expr.Payload{
			DestRegister: 1,
			Base:         expr.PayloadBaseNetworkHeader,
			Offset:       offset,
			Len:          uint32(ipsize),
		},
		&expr.Bitwise{
			SourceRegister: 1,
			DestRegister:   1,
			Len:            uint32(ipsize),
			Mask:           ipnet.Mask,
			Xor:            xormask,
		},
		&expr.Cmp{
			Op:       expr.CmpOpEq,
			Register: 1,
			Data:     ip,
		},
		&expr.Verdict{
			Kind: expr.VerdictAccept,
		},
	}

}

//export NetfilterAllowPrefix
func NetfilterAllowPrefix(prefix string) int32 {
	_, ipnet, err := net.ParseCIDR(prefix)
	if err != nil {
		log.Println("Unable to parse", prefix)
		return -1
	}

	log.Println("Allow traffic from", prefix)

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: buildIpNetExpr(ipnet, DADDR),
	})

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.input,
		Exprs: buildIpNetExpr(ipnet, SADDR),
	})

	return mozvpn_ctx.nftCommit()
}

//export NetfilterMarkInbound
func NetfilterMarkInbound(ipaddr string) int32 {
	element := []nftables.SetElement{
		{Key: net.ParseIP(ipaddr).To4()},
	}

	mozvpn_ctx.conn.SetAddElements(mozvpn_ctx.addrset, element)

	log.Println("Marking inbound traffic from server")
	return mozvpn_ctx.nftCommit()
}

//export NetfilterClearInbound
func NetfilterClearInbound(ipaddr string) int32 {
	element := []nftables.SetElement{
		{Key: net.ParseIP(ipaddr).To4()},
	}

	mozvpn_ctx.conn.SetDeleteElements(mozvpn_ctx.addrset, element)
	log.Println("Clearing traffic marks for server")
	return mozvpn_ctx.nftCommit()
}

//export NetfilterIsolateIpv6
func NetfilterIsolateIpv6(ifname string, ipv6addr string) int32 {
	// Inbound packets from any interface other than the tunnel should
	// be dropped if they are routed to an address assigned to the tunnel.
	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.preroute,
		Exprs: []expr.Any{
			// Match IPv6 packets only
			&expr.Meta{
				Key:      expr.MetaKeyNFPROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.NFPROTO_IPV6},
			},
			// Match packets arriving from outside the tunnel
			&expr.Meta{
				Key:      expr.MetaKeyIIFNAME,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpNeq,
				Register: 1,
				Data:     nftIfname(ifname),
			},
			// Match packets sent from non-local addresses.
			&expr.Fib{
				Register:       1,
				ResultADDRTYPE: true,
				FlagSADDR:      true,
			},
			&expr.Cmp{
				Op:       expr.CmpOpNeq,
				Register: 1,
				Data:     binaryutil.NativeEndian.PutUint32(linux.RTN_LOCAL),
			},
			// Match packets sent to the IPv6 address
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseNetworkHeader,
				Offset:       24,
				Len:          16,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     net.ParseIP(ipv6addr).To16(),
			},
			// Drop the packets
			&expr.Verdict{
				Kind: expr.VerdictDrop,
			},
		},
	})

	log.Println("Isolating tunnel address", ipv6addr)
	return mozvpn_ctx.nftCommit()
}

//export NetfilterMarkCgroupV1
func NetfilterMarkCgroupV1(cgroup uint32) int32 {
	if mozvpn_ctx.fwmark == 0 {
		log.Println("Unable to mark traffic: no fwmark")
		return -1
	}

	mozvpn_ctx.nftMarkCgroup1netcls(cgroup)

	log.Println("Marking traffic from cgroup", cgroup)
	return mozvpn_ctx.nftCommit()
}

//export NetfilterMarkCgroupV2
func NetfilterMarkCgroupV2(cgroup string) int32 {
	if mozvpn_ctx.fwmark == 0 {
		log.Println("Unable to mark traffic: no fwmark")
		return -1
	}

	log.Println("Marking traffic from cgroup", cgroup)
	mozvpn_ctx.nftMarkCgroup2xt(cgroup)

	return mozvpn_ctx.nftCommit()
}

//export NetfilterResetCgroupV2
func NetfilterResetCgroupV2(cgroup string) int32 {
	xtcgroup := nftXtCgroupMatch(cgroup)

	// Delete all mangle rules matching against the cgroup.
	rules, err := mozvpn_ctx.conn.GetRules(mozvpn_ctx.table, mozvpn_ctx.cgroup_mark)
	if err != nil {
		log.Println("Failed to inspect inet/mangle rules", err)
	} else {
		mozvpn_ctx.nftDelCgroup2xt(rules, &xtcgroup)
	}

	// Delete all NAT rules matching against the cgroup.
	rules, err = mozvpn_ctx.conn.GetRules(mozvpn_ctx.table, mozvpn_ctx.cgroup_nat)
	if err != nil {
		log.Println("Failed to inspect inet/nat rules", err)
	} else {
		mozvpn_ctx.nftDelCgroup2xt(rules, &xtcgroup)
	}

	return mozvpn_ctx.nftCommit()
}

//export NetfilterResetAllCgroupsV2
func NetfilterResetAllCgroupsV2() int32 {
	log.Println("Clearing all cgroup traffic marks")
	mozvpn_ctx.conn.FlushChain(mozvpn_ctx.cgroup_mark)
	mozvpn_ctx.conn.FlushChain(mozvpn_ctx.cgroup_nat)
	return mozvpn_ctx.nftCommit()
}

const (
	ROUTER_SOLICITATION_OUT_DST_ADDR = "ff02::2/128"
	SOLICITED_NODE_MULTICAST         = "ff02::1:ff00:0/104"
	IPV6_LINK_LOCAL                  = "fe80::/10"
)

//export NetfilterAllowNDP
func NetfilterAllowNDP() int32 {
	// Output

	_, ipnet, err := net.ParseCIDR(ROUTER_SOLICITATION_OUT_DST_ADDR)
	if err != nil {
		log.Println("UNEXPECTED: Unable to parse hardcoded IP range", ROUTER_SOLICITATION_OUT_DST_ADDR)
		return -1
	}

	ip6rule := buildIpNetExpr(ipnet, DADDR)
	// Remove the verdict, we are going to add more stuff to the slice first.
	ip6rule = ip6rule[:len(ip6rule)-1]

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(133)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-11
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	_, ipnet, err = net.ParseCIDR(SOLICITED_NODE_MULTICAST)
	if err != nil {
		log.Println("UNEXPECTED: Unable to parse hardcoded IP range", SOLICITED_NODE_MULTICAST)
		return -1
	}

	ip6rule = buildIpNetExpr(ipnet, DADDR)
	// Remove the verdict, we are going to add more stuff to the slice first.
	ip6rule = ip6rule[:len(ip6rule)-1]

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(135)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-13
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	_, ipnet, err = net.ParseCIDR(IPV6_LINK_LOCAL)
	if err != nil {
		log.Println("UNEXPECTED: Unable to parse hardcoded IP range", IPV6_LINK_LOCAL)
		return -1
	}

	ip6rule = buildIpNetExpr(ipnet, DADDR)
	// Remove the verdict, we are going to add more stuff to the slice first.
	ip6rule = ip6rule[:len(ip6rule)-1]

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(135)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-13
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(136)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-14
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	// Input

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.input,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(134)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-12
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.input,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(135)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-13
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.input,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(136)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-14
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.input,
		Exprs: append(ip6rule, []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_ICMPV6},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // type
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(137)}, // https://www.iana.org/assignments/icmpv6-parameters/icmpv6-parameters.xhtml#icmpv6-parameters-codes-15
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(1), // code
				Len:          uint32(1),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{uint8(0)},
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		}...),
	})

	return 0
}

const (
	DHCPV4_SERVER_PORT = 67
	DHCPV4_CLIENT_PORT = 68
	DNS_PORT           = 53
)

//export NetfilterAllowDHCP
func NetfilterAllowDHCP() int32 {
	// Output

	// udp sport 68 ip daddr 255.255.255.255 udp dport 67 accept
	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // sport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_CLIENT_PORT),
			},
			&expr.Meta{
				Key:      expr.MetaKeyNFPROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.NFPROTO_IPV4},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseNetworkHeader,
				Offset:       uint32(16), // daddr
				Len:          uint32(4),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{0xFF, 0xFF, 0xFF, 0xFF}, // 255.255.255.255 (Broadcast)
			},
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(2), // dport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_SERVER_PORT),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	// udp sport 67 udp dport 68 accept
	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // sport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_SERVER_PORT),
			},
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(2), // dport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_CLIENT_PORT),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	// Input

	// udp sport 68 ip daddr 255.255.255.255 udp dport 67 accept
	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.input,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // sport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_CLIENT_PORT),
			},
			&expr.Meta{
				Key:      expr.MetaKeyNFPROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.NFPROTO_IPV4},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseNetworkHeader,
				Offset:       uint32(16), // daddr
				Len:          uint32(4),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{0xFF, 0xFF, 0xFF, 0xFF}, // 255.255.255.255 (Broadcast)
			},
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(2), // dport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_SERVER_PORT),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.input,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(0), // sport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_CLIENT_PORT),
			},
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(2), // dport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DHCPV4_SERVER_PORT),
			},
			&expr.Verdict{
				Kind: expr.VerdictAccept,
			},
		},
	})

	return mozvpn_ctx.nftCommit()
}

//export NetfilterBlockDNS
func NetfilterBlockDNS() int32 {
	// udp dport 53 reject
	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_UDP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(2), // dport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DNS_PORT),
			},
			&expr.Reject{
				Type: uint32(linux.NFT_REJECT_ICMPX_UNREACH),
				Code: uint8(linux.NFT_REJECT_ICMPX_PORT_UNREACH),
			},
		},
	})

	// tcp dport 53 reject with tcp reset
	mozvpn_ctx.conn.AddRule(&nftables.Rule{
		Table: mozvpn_ctx.table,
		Chain: mozvpn_ctx.output,
		Exprs: []expr.Any{
			&expr.Meta{
				Key:      expr.MetaKeyL4PROTO,
				Register: 1,
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     []byte{linux.IPPROTO_TCP},
			},
			&expr.Payload{
				DestRegister: 1,
				Base:         expr.PayloadBaseTransportHeader,
				Offset:       uint32(2), // dport
				Len:          uint32(2),
			},
			&expr.Cmp{
				Op:       expr.CmpOpEq,
				Register: 1,
				Data:     binaryutil.BigEndian.PutUint16(DNS_PORT),
			},
			&expr.Reject{
				Type: uint32(linux.NFT_REJECT_TCP_RST),
			},
		},
	})

	return mozvpn_ctx.nftCommit()
}

func main() {}
