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
}

var mozvpn_conn = nftables.Conn{}
var mozvpn_ctx_v4 = nftCtx{}
var mozvpn_ctx_v6 = nftCtx{}

func nftCommit(logger *log.Logger) int32 {
  if err := mozvpn_conn.Flush(); err != nil {
    logger.Println("Netfiler commit failed:", err)
    return -1
  }
  return 0
}

func nftMarkCgroup(ctx *nftCtx, cgroup uint32, fwmark uint32) {
  // Match packets originating from the cgroup
  var loadcgroup = expr.Meta{
    Key:        expr.MetaKeyCGROUP,
    Register:   1,
  }
  var matchcgroup = expr.Cmp{
    Op:         expr.CmpOpEq,
    Register:   1,
    Data:       binaryutil.NativeEndian.PutUint32(cgroup),
  }
  // Match packets that have not been marked
  var loadfwmark = expr.Meta{
    Key:        expr.MetaKeyMARK,
    Register:   1,
  }
  var matchfwmark = expr.Cmp{
    Op:         expr.CmpOpNeq,
    Register:   1,
    Data:       binaryutil.NativeEndian.PutUint32(fwmark),
  }
  // Set the packet mark
  var immfwmark = expr.Immediate{
    Register:   1,
    Data:       binaryutil.NativeEndian.PutUint32(fwmark),
  }
  var setfwmark = expr.Meta{
    Key:        expr.MetaKeyMARK,
    Register:   1,
    SourceRegister: true,
  }

  if ctx.table.Family == nftables.TableFamilyIPv6 {
    mozvpn_conn.AddRule(&nftables.Rule{
      Table: ctx.table,
      Chain: ctx.mangle,
      Exprs: []expr.Any{
        // Load the boilerplate matching rules
        &loadcgroup,
        &matchcgroup,
        &loadfwmark,
        &matchfwmark,
        // Match packets sent from IPv6 localhost
        &expr.Payload{
          DestRegister: 1,
          Base:       expr.PayloadBaseNetworkHeader,
          Offset:     24,
          Len:        16,
        },
        &expr.Cmp{
          Op:         expr.CmpOpNeq,
          Register:   1,
          Data:       net.ParseIP("::1").To16(),
        },
        // Set the firewall mark
        &immfwmark,
        &setfwmark,
      },
    })
  } else {
    mozvpn_conn.AddRule(&nftables.Rule{
      Table: ctx.table,
      Chain: ctx.mangle,
      Exprs: []expr.Any{
        // Load the boilerplate matching rules
        &loadcgroup,
        &matchcgroup,
        &loadfwmark,
        &matchfwmark,
        // Match packets sent from 127.0.0.0/8
        &expr.Payload{
          DestRegister: 1,
          Base:       expr.PayloadBaseNetworkHeader,
          Offset:     16,
          Len:        4,
        },
        &expr.Bitwise{
          SourceRegister: 1,
          DestRegister: 1,
          Len:        4,
          Mask:       net.ParseIP("255.0.0.0").To4(),
          Xor:        []byte{0,0,0,0},
        },
        &expr.Cmp{
          Op:         expr.CmpOpNeq,
          Register:   1,
          Data:       net.ParseIP("127.0.0.0").To4(),
        },
        // Set the firewall mark
        &immfwmark,
        &setfwmark,
      },
    })
  }

  mozvpn_conn.AddRule(&nftables.Rule{
    Table: ctx.table,
    Chain: ctx.nat,
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
  // Destination IP address parsing is a little different for IPv4 and IPv6
  var localhost []byte
  var dstoffset uint32
  if ctx.table.Family == nftables.TableFamilyIPv6 {
    localhost = net.ParseIP("::1").To16()
    dstoffset = 24
  } else {
    localhost = net.ParseIP("127.0.0.1").To4()
    dstoffset = 16
  }

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
      // Match all destinations except localhost
      &expr.Payload{
        DestRegister: 1,
        Base:       expr.PayloadBaseNetworkHeader,
        Offset:     dstoffset,
        Len:        uint32(len(localhost)),
      },
      &expr.Cmp{
        Op:         expr.CmpOpNeq,
        Register:   1,
        Data:       localhost,
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

  mozvpn_ctx_v4.table = mozvpn_conn.AddTable(&nftables.Table{
    Family: nftables.TableFamilyIPv4,
    Name: "mozvpn-v4",
  })
  mozvpn_ctx_v4.mangle = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-mangle-v4",
    Table:      mozvpn_ctx_v4.table,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityMangle,
  })
  mozvpn_ctx_v4.nat = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-nat-v4",
    Table:      mozvpn_ctx_v4.table,
    Type:       nftables.ChainTypeNAT,
    Hooknum:    nftables.ChainHookPostrouting,
    Priority:   nftables.ChainPriorityNATSource,
  })

  mozvpn_ctx_v6.table = mozvpn_conn.AddTable(&nftables.Table{
    Family:     nftables.TableFamilyIPv6,
    Name:       "mozvpn-v6",
  })
  mozvpn_ctx_v6.mangle = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-mangle-v6",
    Table:      mozvpn_ctx_v6.table,
    Type:       nftables.ChainTypeRoute,
    Hooknum:    nftables.ChainHookOutput,
    Priority:   nftables.ChainPriorityMangle,
  })
  mozvpn_ctx_v6.nat = mozvpn_conn.AddChain(&nftables.Chain{
    Name:       "mozvpn-nat-v6",
    Table:      mozvpn_ctx_v6.table,
    Type:       nftables.ChainTypeNAT,
    Hooknum:    nftables.ChainHookPostrouting,
    Priority:   nftables.ChainPriorityNATSource,
  })

  logger.Println("Creating netfilter tables")
  return nftCommit(logger)
}

//export NetfilterRemoveTables
func NetfilterRemoveTables() int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)
  
  mozvpn_conn.DelTable(mozvpn_ctx_v4.table)
  mozvpn_conn.DelTable(mozvpn_ctx_v6.table)

  logger.Println("Removing netfilter tables")
  return nftCommit(logger)
}

//export NetfilterClearTables
func NetfilterClearTables() int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  mozvpn_conn.FlushChain(mozvpn_ctx_v4.mangle)
  mozvpn_conn.FlushChain(mozvpn_ctx_v4.nat)
  mozvpn_conn.FlushChain(mozvpn_ctx_v6.mangle)
  mozvpn_conn.FlushChain(mozvpn_ctx_v6.nat)

  logger.Println("Clearing netfilter tables")
  return nftCommit(logger)
}

//export NetfilterMarkCgroup
func NetfilterMarkCgroup(cgroup uint32, fwmark uint32) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  nftMarkCgroup(&mozvpn_ctx_v4, cgroup, fwmark)
  nftMarkCgroup(&mozvpn_ctx_v6, cgroup, fwmark)

  logger.Println("Marking traffic from cgroup", cgroup)
  return nftCommit(logger)
}

//export NetfilterBlockCgroup
func NetfilterBlockCgroup(cgroup uint32) int32 {
  logger := log.New(&CLogger{level: 0}, "", 0)

  nftBlockCgroup(&mozvpn_ctx_v4, cgroup)
  nftBlockCgroup(&mozvpn_ctx_v6, cgroup)

  logger.Println("Blocking traffic from cgroup", cgroup)
  return nftCommit(logger)
}

func main() {}
