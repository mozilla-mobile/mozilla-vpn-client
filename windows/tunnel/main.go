/* SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2019 Edge Security LLC. All Rights Reserved.
 */

package main

// #include <stdlib.h>
// #include <sys/types.h>
// static void callLogger(void *func, int level, const char *msg)
// {
// 	((void(*)(int, const char *))func)(level, msg);
// }
import "C"

import (
	"log"
	"path/filepath"

	"C"

	"golang.zx2c4.com/wireguard/windows/conf"
	"golang.zx2c4.com/wireguard/windows/tunnel"
	"golang.zx2c4.com/wireguard/windows/tunnel/firewall"

	"errors"
	"unsafe"
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

//export WireGuardTunnelLogger
func WireGuardTunnelLogger(loggerFn uintptr) {
	loggerFunc = unsafe.Pointer(loggerFn)
}

//export WireGuardTunnelService
func WireGuardTunnelService(confFile string) bool {
	logger := log.New(&CLogger{level: 0}, "", 0);

	tunnel.UseFixedGUIDInsteadOfDeterministic = true
	firewall.ExemptBuiltinAdministrators = true

	conf.PresetRootDirectory(filepath.Dir(confFile))

	err := tunnel.Run(confFile)
	if err != nil {
		logger.Println("Tunnel service error: %v", err)
	}

	return err == nil
}

func main() {}
