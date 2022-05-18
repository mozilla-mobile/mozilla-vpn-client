/* SPDX-License-Identifier: MIT
 *
 * Copyright (C) 2019-2021 WireGuard LLC. All Rights Reserved.
 */

package main

import (
	"C"

	"golang.org/x/crypto/curve25519"
	"golang.org/x/sys/windows"


	"crypto/rand"
	"log"
	"unsafe"
)

//export WireGuardTunnelService
func WireGuardTunnelService(confString16 *uint16, nameString16 *uint16) bool {
	confStr := windows.UTF16PtrToString(confString16)
	nameStr := windows.UTF16PtrToString(nameString16)
	UseFixedGUIDInsteadOfDeterministic = true
	err := Run(confStr,nameStr)
	if err != nil {
		log.Printf("Service run error: %v", err)
	}
	return err == nil
}

//export WireGuardGenerateKeypair
func WireGuardGenerateKeypair(publicKey *byte, privateKey *byte) {
	publicKeyArray := (*[32]byte)(unsafe.Pointer(publicKey))
	privateKeyArray := (*[32]byte)(unsafe.Pointer(privateKey))
	n, err := rand.Read(privateKeyArray[:])
	if err != nil || n != len(privateKeyArray) {
		panic("Unable to generate random bytes")
	}
	privateKeyArray[0] &= 248
	privateKeyArray[31] = (privateKeyArray[31] & 127) | 64

	curve25519.ScalarBaseMult(publicKeyArray, privateKeyArray)
}

func main() {}
