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

var loggerFunc unsafe.Pointer

type CLogger struct {
  level C.int
}

func (l *CLogger) Write(p []byte) (int, error) {
  if uintptr(loggerFunc) == 0 {
    return 0, errors.New("No logger initialized")
  }
  message := C.CString(string(p))
  C.callLogger(loggerFunc, l.level, message)
  C.free(unsafe.Pointer(message))
  return len(p), nil
}

//export NetfilterSetLogger
func NetfilterSetLogger(loggerFn uintptr) {
  loggerFunc = unsafe.Pointer(loggerFn)
}

type nftCtx struct {
  table_inet  *nftables.Table
  table_v6    *nftables.Table
  mangle      *nftables.Chain
  nat         *nftables.Chain
  conntrack   *nftables.Chain
  preroute    *nftables.Chain
  preroute_v6 *nftables.Chain
}

var mozvpn_conn = nftables.Conn{}
var mozvpn_ctx = nftCtx{}

func nftCommit(logger *log.Logger) int32 {
  if err := mozvpn_conn.Flush(); err != nil {
    logger.Println("Netfiler commit failed:", err)
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

// A firewall mark is used to flag packets that need to be NAT-ed
// for re-routing after they have passed through connection tracking.
const fwmark_needs_nat = 0xe253

func nftIfup(ctx *nftCtx, ifname string, fwmark uint32) {
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
  mozvpn_conn.AddRule(&nftables.Rule{
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
        Data:       binaryutil.NativeEndian.PutUint32(fwmark),
      },
      // Set the conntrack zone
      &immctzone,
      &setctzone,
    },
  })

  // Inbound packets from outside the tunnel should be marked for RPF
  // and moved into the external conntrack zone.
  mozvpn_conn.AddRule(&nftables.Rule{
    Table: ctx.table_inet,
    Chain: ctx.preroute,
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
      // Match packets arriving on non-localhost.
			&expr.Meta{
        Key:      expr.MetaKeyIIFTYPE,
        Register: 1,
      },
      &expr.Cmp{
        Op:       expr.CmpOpNeq,
        Register: 1,
        Data:     binaryutil.NativeEndian.PutUint16(linux.ARPHRD_LOOPBACK),
      },
      // Set the firewall mark for the packets
      &expr.Immediate{
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(fwmark),
      },
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
        SourceRegister: true,
      },
      // Set the conntrack zone
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

func nftMarkCgroup2xt(ctx *nftCtx, cgroup string, logger *log.Logger) {
  xtmatch := nftXtCgroupMatch(cgroup)

  mozvpn_conn.AddRule(&nftables.Rule{
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
        Data:       binaryutil.NativeEndian.PutUint32(fwmark_needs_nat),
      },
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
        SourceRegister: true,
      },
    },
  })
}

func nftMarkCgroup1netcls(ctx *nftCtx, cgroup uint32, fwmark uint32) {
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

  mozvpn_conn.AddRule(&nftables.Rule{
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
        Data:       binaryutil.NativeEndian.PutUint32(fwmark),
      },
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
        SourceRegister: true,
      },
    },
  })

  // Masquerade outgoing packets from split tunnelling to trigger rerouting.
  mozvpn_conn.AddRule(&nftables.Rule{
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
        Data:       binaryutil.NativeEndian.PutUint32(fwmark),
      },
      // Masquerade
      &expr.Masq{},
    },
  })
}

func nftBlockCgroup(ctx *nftCtx, cgroup uint32) {
  mozvpn_conn.AddRule(&nftables.Rule{
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
  logger := log.New(&CLogger{level: 0}, "", 0)

  mozvpn_ctx.table_inet = mozvpn_conn.AddTable(&nftables.Table{
    Family: nftables.TableFamilyINet,
    Name: "mozvpn-inet",
  })
  mozvpn_ctx.mangle = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-mangle",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityRaw,
  })
  mozvpn_ctx.nat = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-nat",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeNAT,
    Hooknum:    nftables.ChainHookPostrouting,
    Priority:   nftables.ChainPriorityNATSource,
  })
  mozvpn_ctx.conntrack = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-conntrack",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityConntrack-1,
  })
  mozvpn_ctx.preroute = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-preroute",
    Table:      mozvpn_ctx.table_inet,
    Type:       nftables.ChainTypeFilter,
    Hooknum:    nftables.ChainHookPrerouting,
    Priority:   nftables.ChainPriorityRaw,
  })

  mozvpn_ctx.table_v6 = mozvpn_conn.AddTable(&nftables.Table{
    Family: nftables.TableFamilyIPv6,
    Name: "mozvpn-v6",
  })
  mozvpn_ctx.preroute_v6 = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-preroute-v6",
    Table:      mozvpn_ctx.table_v6,
    Type:       nftables.ChainTypeFilter,
    Hooknum:    nftables.ChainHookPrerouting,
    Priority:   nftables.ChainPriorityRaw,
  })

  logger.Println("Creating netfilter tables")
  return nftCommit(logger)
}

//export NetfilterRemoveTables
func NetfilterRemoveTables() int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)
  
  mozvpn_conn.DelTable(mozvpn_ctx.table_inet)
  mozvpn_conn.DelTable(mozvpn_ctx.table_v6)

  logger.Println("Removing netfilter tables")
  return nftCommit(logger)
}

//export NetfilterClearTables
func NetfilterClearTables() int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  mozvpn_conn.FlushChain(mozvpn_ctx.mangle)
  mozvpn_conn.FlushChain(mozvpn_ctx.nat)
  mozvpn_conn.FlushChain(mozvpn_ctx.conntrack)
  mozvpn_conn.FlushChain(mozvpn_ctx.preroute)
  mozvpn_conn.FlushChain(mozvpn_ctx.preroute_v6)

  // TODO: Delete everything in mozvpn_ctx.cgroup_map

  logger.Println("Clearing netfilter tables")
  return nftCommit(logger)
}

//export NetfilterIfup
func NetfilterIfup(ifname string, fwmark uint32) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  if fwmark != 0 {
    nftIfup(&mozvpn_ctx, ifname, fwmark)
  }

  logger.Println("Starting netfilter tables for", ifname)
  return nftCommit(logger)
}

//export NetfilterIsolateIpv6
func NetfilterIsolateIpv6(ifname string, ipv6addr string) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  // Inbound packets from any interface other than the tunnel should
  // be dropped if they are routed to an address assigned to the tunnel.
  mozvpn_conn.AddRule(&nftables.Rule{
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

  logger.Println("Isolating tunnel address", ipv6addr)
  return nftCommit(logger)
}

//export NetfilterMarkCgroupV1
func NetfilterMarkCgroupV1(cgroup uint32, fwmark uint32) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  nftMarkCgroup1netcls(&mozvpn_ctx, cgroup, fwmark)

  logger.Println("Marking traffic from cgroup", cgroup)
  return nftCommit(logger)
}

//export NetfilterMarkCgroupV2
func NetfilterMarkCgroupV2(cgroup string) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  logger.Println("Marking traffic from cgroup", cgroup)
  nftMarkCgroup2xt(&mozvpn_ctx, cgroup, logger)

  return nftCommit(logger)
}

//export NetfilterResetCgroupV2
func NetfilterResetCgroupV2(cgroup string) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  // Start by dumping the mangle rules. We will search for matching
  // cgroup/masquerade rules and remove them by handle.
  rules, err := mozvpn_conn.GetRules(mozvpn_ctx.table_inet, mozvpn_ctx.mangle)
  if err != nil {
    logger.Println("Failed to inspect inet/mangle rules", err)
    return 1
  }

  // Search for a mangle rule starting with a cgroup match.
  xtcgroup := nftXtCgroupMatch(cgroup)
  for _, r := range rules {
    rr, _ := r.Exprs[0].(*expr.Match)
    if *rr == xtcgroup {
      logger.Println("Deleting inet/mangle rule", r.Handle)
      mozvpn_conn.DelRule(r);
    }
  }
  
  return nftCommit(logger)
}

//export NetfilterResetAllCgroupsV2
func NetfilterResetAllCgroupsV2() int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  logger.Println("Clearing all cgroup traffic marks")
  mozvpn_conn.FlushChain(mozvpn_ctx.mangle)

  return nftCommit(logger)
}

func main() {}
