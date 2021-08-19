/* SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2019-2021 WireGuard LLC. All Rights Reserved.
 */

package main

import (
	"bytes"
	"log"
	"net"
	"sort"

	"golang.org/x/sys/windows"
	"golang.zx2c4.com/wireguard/tun"

	"golang.zx2c4.com/wireguard/windows/conf"
	"golang.zx2c4.com/wireguard/windows/tunnel/firewall"
	"golang.zx2c4.com/wireguard/windows/tunnel/winipcfg"
)

func cleanupAddressesOnDisconnectedInterfaces(family winipcfg.AddressFamily, addresses []net.IPNet) {
	if len(addresses) == 0 {
		return
	}
	includedInAddresses := func(a net.IPNet) bool {
		// TODO: this makes the whole algorithm O(n^2). But we can't stick net.IPNet in a Go hashmap. Bummer!
		for _, addr := range addresses {
			ip := addr.IP
			if ip4 := ip.To4(); ip4 != nil {
				ip = ip4
			}
			mA, _ := addr.Mask.Size()
			mB, _ := a.Mask.Size()
			if bytes.Equal(ip, a.IP) && mA == mB {
				return true
			}
		}
		return false
	}
	interfaces, err := winipcfg.GetAdaptersAddresses(family, winipcfg.GAAFlagDefault)
	if err != nil {
		return
	}
	for _, iface := range interfaces {
		if iface.OperStatus == winipcfg.IfOperStatusUp {
			continue
		}
		for address := iface.FirstUnicastAddress; address != nil; address = address.Next {
			ip := address.Address.IP()
			ipnet := net.IPNet{IP: ip, Mask: net.CIDRMask(int(address.OnLinkPrefixLength), 8*len(ip))}
			if includedInAddresses(ipnet) {
				log.Printf("Cleaning up stale address %s from interface ‘%s’", ipnet.String(), iface.FriendlyName())
				iface.LUID.DeleteIPAddress(ipnet)
			}
		}
	}
}

func configureInterface(family winipcfg.AddressFamily, conf *conf.Config, tun *tun.NativeTun) error {
	luid := winipcfg.LUID(tun.LUID())

	estimatedRouteCount := 0
	for _, peer := range conf.Peers {
		estimatedRouteCount += len(peer.AllowedIPs)
	}
	routes := make([]winipcfg.RouteData, 0, estimatedRouteCount)
	addresses := make([]net.IPNet, len(conf.Interface.Addresses))
	var haveV4Address, haveV6Address bool
	for i, addr := range conf.Interface.Addresses {
		addresses[i] = addr.IPNet()
		if addr.Bits() == 32 {
			haveV4Address = true
		} else if addr.Bits() == 128 {
			haveV6Address = true
		}
	}

	foundDefault4 := false
	foundDefault6 := false
	for _, peer := range conf.Peers {
		for _, allowedip := range peer.AllowedIPs {
			allowedip.MaskSelf()
			if (allowedip.Bits() == 32 && !haveV4Address) || (allowedip.Bits() == 128 && !haveV6Address) {
				continue
			}
			route := winipcfg.RouteData{
				Destination: allowedip.IPNet(),
				Metric:      0,
			}
			if allowedip.Bits() == 32 {
				if allowedip.Cidr == 0 {
					foundDefault4 = true
				}
				route.NextHop = net.IPv4zero
			} else if allowedip.Bits() == 128 {
				if allowedip.Cidr == 0 {
					foundDefault6 = true
				}
				route.NextHop = net.IPv6zero
			}
			routes = append(routes, route)
		}
	}

	err := luid.SetIPAddressesForFamily(family, addresses)
	if err == windows.ERROR_OBJECT_ALREADY_EXISTS {
		cleanupAddressesOnDisconnectedInterfaces(family, addresses)
		err = luid.SetIPAddressesForFamily(family, addresses)
	}
	if err != nil {
		return err
	}

	deduplicatedRoutes := make([]*winipcfg.RouteData, 0, len(routes))
	sort.Slice(routes, func(i, j int) bool {
		if routes[i].Metric != routes[j].Metric {
			return routes[i].Metric < routes[j].Metric
		}
		if c := bytes.Compare(routes[i].NextHop, routes[j].NextHop); c != 0 {
			return c < 0
		}
		if c := bytes.Compare(routes[i].Destination.IP, routes[j].Destination.IP); c != 0 {
			return c < 0
		}
		if c := bytes.Compare(routes[i].Destination.Mask, routes[j].Destination.Mask); c != 0 {
			return c < 0
		}
		return false
	})
	for i := 0; i < len(routes); i++ {
		if i > 0 && routes[i].Metric == routes[i-1].Metric &&
			bytes.Equal(routes[i].NextHop, routes[i-1].NextHop) &&
			bytes.Equal(routes[i].Destination.IP, routes[i-1].Destination.IP) &&
			bytes.Equal(routes[i].Destination.Mask, routes[i-1].Destination.Mask) {
			continue
		}
		deduplicatedRoutes = append(deduplicatedRoutes, &routes[i])
	}

	if !conf.Interface.TableOff {
		err = luid.SetRoutesForFamily(family, deduplicatedRoutes)
		if err != nil {
			return err
		}
	}

	ipif, err := luid.IPInterface(family)
	if err != nil {
		return err
	}
	if conf.Interface.MTU > 0 {
		ipif.NLMTU = uint32(conf.Interface.MTU)
		tun.ForceMTU(int(ipif.NLMTU))
	}
	if family == windows.AF_INET {
		if foundDefault4 {
			ipif.UseAutomaticMetric = false
			ipif.Metric = 0
		}
	} else if family == windows.AF_INET6 {
		if foundDefault6 {
			ipif.UseAutomaticMetric = false
			ipif.Metric = 0
		}
		ipif.DadTransmits = 0
		ipif.RouterDiscoveryBehavior = winipcfg.RouterDiscoveryDisabled
	}
	err = ipif.Set()
	if err != nil {
		return err
	}

	return luid.SetDNS(family, conf.Interface.DNS, conf.Interface.DNSSearch)
}

func enableFirewall(conf *conf.Config, tun *tun.NativeTun) error {
	doNotRestrict := true
	if len(conf.Peers) == 1 && !conf.Interface.TableOff {
	nextallowedip:
		for _, allowedip := range conf.Peers[0].AllowedIPs {
			if allowedip.Cidr == 0 {
				for _, b := range allowedip.IP {
					if b != 0 {
						continue nextallowedip
					}
				}
				doNotRestrict = false
				break
			}
		}
	}
	log.Println("Enabling firewall rules")
	return firewall.EnableFirewall(tun.LUID(), doNotRestrict, conf.Interface.DNS)
}
