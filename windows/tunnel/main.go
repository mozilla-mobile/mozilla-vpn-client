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
	"fmt"
	"io/ioutil"
	"log"
	"net"
	"net/http"
	"path/filepath"
	"strings"

	"C"

	"golang.org/x/sys/windows"

	"golang.zx2c4.com/wireguard/windows/conf"
	"golang.zx2c4.com/wireguard/windows/tunnel"
	"golang.zx2c4.com/wireguard/windows/tunnel/winipcfg"
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

func findPhysicalDefaultRoute() (winipcfg.LUID, net.IP, error) {
	r, err := winipcfg.GetIPForwardTable2(windows.AF_INET)
	if err != nil {
		return 0, nil, err
	}

	lowestMetric := ^uint32(0)

	var nextHop net.IP
	var luid winipcfg.LUID
	for i := range r {
		if r[i].DestinationPrefix.PrefixLength != 0 {
			continue
		}
		ifrow, err := r[i].InterfaceLUID.Interface()
		if err != nil || ifrow.OperStatus != winipcfg.IfOperStatusUp || ifrow.MediaType == winipcfg.NdisMediumIP {
			continue
		}
		if r[i].Metric < lowestMetric {
			lowestMetric = r[i].Metric
			nextHop = r[i].NextHop.IP()
			luid = r[i].InterfaceLUID
		}
	}

	if len(nextHop) == 0 {
		return 0, nil, errors.New("Unable to find default route")
	}

	return luid, nextHop, nil
}

//export WireguardTestOutsideConnectivity
func WireguardTestOutsideConnectivity(ip string, host string, url string, expectedTestResult string) int32 {
	logger := log.New(&CLogger{level: 0}, "", 0);

	// Attempt to locate a default route
	luid, nextHop, err := findPhysicalDefaultRoute()
	if err != nil {
		logger.Println("Failed to find the physical default route: %v", err);
		return -1
	}

	destination := net.IPNet{IP: net.ParseIP(ip), Mask: net.IPv4Mask(255, 255, 255, 255)}
	err = luid.AddRoute(destination, nextHop, 0)

	// Check for errors, and check if route already exists
	if err != nil && err != windows.ERROR_OBJECT_ALREADY_EXISTS {
		logger.Println("Failed to add a new route: %v", err);
		return -1
	}
	defer luid.DeleteRoute(destination, nextHop)

	// Attempt to setup a new GET request
	req, err := http.NewRequest("GET", fmt.Sprintf(url, ip), nil)
	if err != nil {
		logger.Println("Failed to create a new HTTP request: %v", err);
		return -1
	}

	// Redirects should be treated as an assumption that a captive portal is available
	redirected := false
	client := &http.Client{
		CheckRedirect: func(r *http.Request, via []*http.Request) error {
			redirected = true
			return errors.New("Redirect detected.")
		},
	}

	// Set the host header and try to retrieve the supplied URL
	req.Host = host
	resp, err := client.Do(req)

	if redirected {
		// We were redirected!
		return 0
	}

	if err != nil {
		logger.Println("Failed to complete the HTTP request: %v", err);
		return -1
	}

	// Read response from the body
	text, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		logger.Println("Failed to read from th HTTP request: %v", err);
		return -1
	}

	// Compare retrieved body contents to expected contents
	compareTestResult := strings.ReplaceAll(strings.ReplaceAll(string(text), "\n", ""), "\r", "")
	if compareTestResult == expectedTestResult {
		return 1
	} else {
		return 0
	}
}

func main() {}
