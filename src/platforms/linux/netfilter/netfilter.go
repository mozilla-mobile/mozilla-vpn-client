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
  "errors"
  "unsafe"

  "C"

  "github.com/google/nftables"
  "github.com/google/nftables/binaryutil"
  "github.com/google/nftables/expr"
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
  table   *nftables.Table
  mangle  *nftables.Chain
  nat     *nftables.Chain
  conntrack *nftables.Chain
  preroute *nftables.Chain
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

const arphdr_loopback = 772
const ctzone_external = 0x9e4c

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
    Table: ctx.table,
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
    Table: ctx.table,
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
        Data:     binaryutil.NativeEndian.PutUint16(arphdr_loopback),
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

func nftMarkCgroup(ctx *nftCtx, cgroup uint32, fwmark uint32) {
  // Match packets originating from the cgroup
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
    Table: ctx.table,
    Chain: ctx.mangle,
    Exprs: []expr.Any{
      // Load the boilerplate matching rules
      &loadcgroup,
      &matchcgroup,
      // Match packets that have not been marked
      &expr.Meta{
        Key:        expr.MetaKeyMARK,
        Register:   1,
      },
      &expr.Cmp{
        Op:         expr.CmpOpNeq,
        Register:   1,
        Data:       binaryutil.NativeEndian.PutUint32(fwmark),
      },
      // Do not match packets sent to localhost interfaces
      &expr.Meta{
        Key:      expr.MetaKeyOIFTYPE,
        Register: 1,
      },
      &expr.Cmp{
        Op:       expr.CmpOpNeq,
        Register: 1,
        Data:     binaryutil.NativeEndian.PutUint16(arphdr_loopback),
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

  // Masquerade outgoing packets from the cgroup to trigger rerouting.
  mozvpn_conn.AddRule(&nftables.Rule{
    Table: ctx.table,
    Chain: ctx.nat,
    Exprs: []expr.Any{
      // Match packets from the cgroup
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
    Table: ctx.table,
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
        Data:     binaryutil.NativeEndian.PutUint16(arphdr_loopback),
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

  mozvpn_ctx.table = mozvpn_conn.AddTable(&nftables.Table{
    Family: nftables.TableFamilyINet,
    Name: "mozvpn",
  })
  mozvpn_ctx.mangle = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-mangle",
    Table:      mozvpn_ctx.table,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityRaw,
  })
  mozvpn_ctx.nat = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-nat",
    Table:      mozvpn_ctx.table,
    Type:       nftables.ChainTypeNAT,
    Hooknum:    nftables.ChainHookPostrouting,
    Priority:   nftables.ChainPriorityNATSource,
  })
  mozvpn_ctx.conntrack = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-conntrack",
    Table:      mozvpn_ctx.table,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityConntrack-1,
  })
  mozvpn_ctx.preroute = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-preroute",
    Table:      mozvpn_ctx.table,
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
  
  mozvpn_conn.DelTable(mozvpn_ctx.table)

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
  return 0
}

//export NetfilterMarkCgroup
func NetfilterMarkCgroup(cgroup uint32, fwmark uint32) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  nftMarkCgroup(&mozvpn_ctx, cgroup, fwmark)

  logger.Println("Marking traffic from cgroup", cgroup)
  return nftCommit(logger)
}

//export NetfilterBlockCgroup
func NetfilterBlockCgroup(cgroup uint32) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  nftBlockCgroup(&mozvpn_ctx, cgroup)

  logger.Println("Blocking traffic from cgroup", cgroup)
  return nftCommit(logger)
}

func main() {}
