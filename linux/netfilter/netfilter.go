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
  "log"
  "net"
  "bytes"
  "errors"
  "unsafe"

  "C"

  "github.com/google/nftables"
  "github.com/google/nftables/alignedbuff"
  "github.com/google/nftables/binaryutil"
  "github.com/google/nftables/expr"
  "github.com/google/nftables/xt"
  linux "golang.org/x/sys/unix"
)

type CLogger struct {
  level C.int
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
    level: 0,
    function: unsafe.Pointer(loggerFn),
  })
  log.SetPrefix("")
  log.SetFlags(0)
}

type nftCtx struct {
  table_inet  *nftables.Table
  table_v6    *nftables.Table
  mangle      *nftables.Chain
  nat         *nftables.Chain
  conntrack   *nftables.Chain
  preroute    *nftables.Chain
  preroute_v6 *nftables.Chain
  addrset     *nftables.Set
  fwmark      uint32
  conn        nftables.Conn
}

var mozvpn_ctx = nftCtx{}

func (ctx* nftCtx) nftCommit() int32 {
  if err := ctx.conn.Flush(); err != nil {
    log.Println("Netfiler commit failed:", err)
    return -1
  }
  return 0
}

func nftIfname(n string) []byte {
  b := make([]byte, 16)
  copy(b, []byte(n+"\x00"))
  return b
}

// A Conntrack zone used for traffic excluded from the VPN tunnel.
// The value is not important, so long as it's constant, unique,
// and non-zero.
const ctzone_external = 0x9e4c

func (ctx* nftCtx) nftIfup(ifname string) {
  var immctzone = expr.Immediate{
    Register:   1,
    Data:       binaryutil.NativeEndian.PutUint32(ctzone_external),
  }
  var setctzone = expr.Ct{
    Key:        expr.CtKeyZONE,
    Register:   1,
    SourceRegister: true,
  }

  // Move all marked packets into their own conntrack zone
  ctx.conn.AddRule(&nftables.Rule{
    Table: ctx.table_inet,
    Chain: ctx.conntrack,
    Exprs: []expr.Any{
      // Match marked packets
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
      },
      &expr.Cmp{
        Op:         expr.CmpOpEq,
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(ctx.fwmark),
      },
      // Set the conntrack zone
      &immctzone,
      &setctzone,
    },
  })

  // Inbound packets from wireguard servers should be marked for RPF
  // and moved into the external conntrack zone.
  ctx.conn.AddRule(&nftables.Rule{
    Table: ctx.table_inet,
    Chain: ctx.preroute,
    Exprs: []expr.Any{
      // For now, we only support IPv4 in this check.
      // TODO: Support IPv6.
      &expr.Meta{
        Key:            expr.MetaKeyPROTOCOL,
        Register:       1,
      },
      &expr.Cmp{
        Op:             expr.CmpOpEq,
        Register:       1,
        Data:           binaryutil.BigEndian.PutUint16(linux.ETH_P_IP),
      },
      // Match UDP packets
      &expr.Meta{
        Key:            expr.MetaKeyL4PROTO,
        Register:       1,
      },
      &expr.Cmp{
        Op:             expr.CmpOpEq,
        Register:       1,
	Data:           []byte{linux.IPPROTO_UDP},
      },
      // Lookup the IPv4 source address.
      &expr.Payload{
        DestRegister:   1,
	Base:           expr.PayloadBaseNetworkHeader,
        Offset:         uint32(12),
        Len:            uint32(4),
      },
      &expr.Lookup{
        SourceRegister: 1,
        SetName:        ctx.addrset.Name,
        SetID:          ctx.addrset.ID,
      },
      // Set the firewall mark.
      &expr.Immediate{
        Register:       1,
        Data:           binaryutil.NativeEndian.PutUint32(ctx.fwmark),
      },
      &expr.Meta{
        Key:            expr.MetaKeyMARK,
        Register:       1,
        SourceRegister: true,
      },
      // Set the conntrack zone.
      &immctzone,
      &setctzone,
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
  xtcgroup.PutUint8(1) // has_path
  xtcgroup.PutUint8(0) // has_classid
  xtcgroup.PutUint8(0) // invert_path
  xtcgroup.PutUint8(0) // invert_classid
  xtcgroup.PutBytesAligned32([]byte(cgroup + "\x00"), 512) // Cgroup path.
  xtcgroup.PutUint64(0) // kernel padding
  var xtinfo = xt.Unknown(xtcgroup.Data())

  return expr.Match{
    Name:     "cgroup",
    Rev:      2,
    Info:     &xtinfo,
  }
}

func (ctx* nftCtx) nftMarkCgroup2xt(cgroup string) {
  xtmatch := nftXtCgroupMatch(cgroup)

  ctx.conn.AddRule(&nftables.Rule{
    Table: ctx.table_inet,
    Chain: ctx.mangle,
    Exprs: []expr.Any{
      // Match the cgroup v2 path for originated packets.
      &xtmatch,
      // Match packets that have not already been marked
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
      },
      &expr.Cmp{
        Op:         expr.CmpOpEq,
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(0),
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
      // Set the firewall mark to request NAT
      &expr.Immediate{
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(ctx.fwmark),
      },
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
        SourceRegister: true,
      },
    },
  })

  // Masquerade outgoing packets from split tunnelling to trigger rerouting.
  ctx.conn.AddRule(&nftables.Rule{
    Table: ctx.table_inet,
    Chain: ctx.nat,
    Exprs: []expr.Any{
      &xtmatch,
      // Match marked packets
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
      },
      &expr.Cmp{
        Op:         expr.CmpOpEq,
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(ctx.fwmark),
      },
      // Masquerade
      &expr.Masq{},
    },
  })
}

func (ctx* nftCtx) nftDelCgroup2xt(rules []*nftables.Rule, xtmatch *expr.Match) {
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
      ctx.conn.DelRule(r);
    }
  }
}

func (ctx* nftCtx) nftMarkCgroup1netcls(cgroup uint32) {
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
    Table: ctx.table_inet,
    Chain: ctx.mangle,
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
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(ctx.fwmark),
      },
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
        SourceRegister: true,
      },
    },
  })

  // Masquerade outgoing packets from split tunnelling to trigger rerouting.
  ctx.conn.AddRule(&nftables.Rule{
    Table: ctx.table_inet,
    Chain: ctx.nat,
    Exprs: []expr.Any{
      &loadcgroup,
      &matchcgroup,
      // Match marked packets
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
      },
      &expr.Cmp{
        Op:         expr.CmpOpEq,
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(ctx.fwmark),
      },
      // Masquerade
      &expr.Masq{},
    },
  })
}

func (ctx* nftCtx) nftBlockCgroup(cgroup uint32) {
  ctx.conn.AddRule(&nftables.Rule{
    Table: ctx.table_inet,
    Chain: ctx.mangle,
    Exprs: []expr.Any{
      // Match packets from the cgroup
      &expr.Meta{
        Key:        expr.MetaKeyCGROUP,
        Register:   1,
      },
      &expr.Cmp{
        Op:         expr.CmpOpEq,
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(cgroup),
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
        Kind:       expr.VerdictDrop,
      },
    },
  })
}

//export NetfilterCreateTables
func NetfilterCreateTables() int32 {
  mozvpn_ctx.table_inet = mozvpn_ctx.conn.AddTable(&nftables.Table{
    Family: nftables.TableFamilyINet,
    Name: "mozvpn-inet",
  })
  mozvpn_ctx.mangle = mozvpn_ctx.conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-mangle",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityRaw,
  })
  mozvpn_ctx.nat = mozvpn_ctx.conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-nat",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeNAT,
    Hooknum:    nftables.ChainHookPostrouting,
    Priority:   nftables.ChainPriorityNATSource,
  })
  mozvpn_ctx.conntrack = mozvpn_ctx.conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-conntrack",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityConntrack-1,
  })
  mozvpn_ctx.preroute = mozvpn_ctx.conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-preroute",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeFilter,
    Hooknum:    nftables.ChainHookPrerouting,
    Priority:   nftables.ChainPriorityRaw,
  })

  mozvpn_ctx.table_v6 = mozvpn_ctx.conn.AddTable(&nftables.Table{
    Family: nftables.TableFamilyIPv6,
    Name: "mozvpn-v6",
  })
  mozvpn_ctx.preroute_v6 = mozvpn_ctx.conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-preroute-v6",
    Table:      mozvpn_ctx.table_v6,
    Type:       nftables.ChainTypeFilter,
    Hooknum:    nftables.ChainHookPrerouting,
    Priority:   nftables.ChainPriorityRaw,
  })

  mozvpn_ctx.addrset = &nftables.Set{
    Table:      mozvpn_ctx.table_inet,
    Name:       "mozvpn-addrset",
    KeyType:    nftables.TypeIPAddr,
  }
  mozvpn_ctx.conn.AddSet(mozvpn_ctx.addrset, nil)

  log.Println("Creating netfilter tables")
  return mozvpn_ctx.nftCommit()
}

//export NetfilterRemoveTables
func NetfilterRemoveTables() int32 {
  mozvpn_ctx.conn.DelTable(mozvpn_ctx.table_inet)
  mozvpn_ctx.conn.DelTable(mozvpn_ctx.table_v6)

  log.Println("Removing netfilter tables")
  return mozvpn_ctx.nftCommit()
}

//export NetfilterClearTables
func NetfilterClearTables() int32 {
  mozvpn_ctx.fwmark = 0
  mozvpn_ctx.conn.FlushChain(mozvpn_ctx.mangle)
  mozvpn_ctx.conn.FlushChain(mozvpn_ctx.nat)
  mozvpn_ctx.conn.FlushChain(mozvpn_ctx.conntrack)
  mozvpn_ctx.conn.FlushChain(mozvpn_ctx.preroute)
  mozvpn_ctx.conn.FlushChain(mozvpn_ctx.preroute_v6)
  mozvpn_ctx.conn.FlushSet(mozvpn_ctx.addrset)

  log.Println("Clearing netfilter tables")
  return mozvpn_ctx.nftCommit()
}

//export NetfilterIfup
func NetfilterIfup(ifname string, fwmark uint32) int32 {
  mozvpn_ctx.fwmark = fwmark
  if fwmark != 0 {
    mozvpn_ctx.nftIfup(ifname)
  }

  log.Println("Starting netfilter tables for", ifname)
  return mozvpn_ctx.nftCommit()
}

//export NetfilterMarkInbound
func NetfilterMarkInbound(ipaddr string, port uint32) int32 {
  element := []nftables.SetElement{
    { Key: net.ParseIP(ipaddr).To4(), },
  }
 
  mozvpn_ctx.conn.SetAddElements(mozvpn_ctx.addrset, element)

  log.Println("Marking inbound traffic from server")
  return mozvpn_ctx.nftCommit()
}

//export NetfilterClearInbound
func NetfilterClearInbound(ipaddr string) int32 {
  element := []nftables.SetElement{
    { Key: net.ParseIP(ipaddr).To4(), },
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
    Table: mozvpn_ctx.table_v6,
    Chain: mozvpn_ctx.preroute_v6,
    Exprs: []expr.Any{
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
        Register: 1,
        ResultADDRTYPE: true,
        FlagSADDR: true,
      },
      &expr.Cmp{
        Op:       expr.CmpOpNeq,
        Register: 1,
        Data:     binaryutil.NativeEndian.PutUint32(linux.RTN_LOCAL),
      },
      // Match packets sent to the IPv6 address
      &expr.Payload{
        DestRegister: 1,
        Base:     expr.PayloadBaseNetworkHeader,
        Offset:   24,
        Len:      16,
      },
      &expr.Cmp{
        Op:       expr.CmpOpEq,
        Register: 1,
        Data:     net.ParseIP(ipv6addr).To16(),
      },
      // Drop the packets
      &expr.Verdict{
        Kind:     expr.VerdictDrop,
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
  rules, err := mozvpn_ctx.conn.GetRules(mozvpn_ctx.table_inet, mozvpn_ctx.mangle)
  if err != nil {
    log.Println("Failed to inspect inet/mangle rules", err)
  } else {
    mozvpn_ctx.nftDelCgroup2xt(rules, &xtcgroup)
  }

  // Delete all NAT rules matching against the cgroup.
  rules, err = mozvpn_ctx.conn.GetRules(mozvpn_ctx.table_inet, mozvpn_ctx.nat)
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
  mozvpn_ctx.conn.FlushChain(mozvpn_ctx.mangle)
  mozvpn_ctx.conn.FlushChain(mozvpn_ctx.nat)
  return mozvpn_ctx.nftCommit()
}

func main() {}
