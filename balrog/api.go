// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

package main

// #include <stdlib.h>
// #include <sys/types.h>
// static void callLogger(void *func, int level, const char *msg)
// {
//   ((void(*)(int, const char *))func)(level, msg);
// }
import "C"

import (
	"crypto/ecdsa"
	"crypto/sha256"
	"crypto/sha512"
	"crypto/x509"
	"encoding/base64"
	"encoding/pem"
	"errors"
	"hash"
	"log"
	"math/big"
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

// ContentSignature contains the parsed representation of a signature
type ContentSignature struct {
	R, S     *big.Int // fields must be exported for ASN.1 marshalling
	HashName string
	Mode     string
	X5U      string
	ID       string
	Len      int
	Finished bool
}

const (
	// Type of this signer is 'contentsignature'
	Type = "contentsignature"

	// P256ECDSA defines an ecdsa content signature on the P-256 curve
	P256ECDSA = "p256ecdsa"

	// P256ECDSABYTESIZE defines the bytes length of a P256ECDSA signature
	P256ECDSABYTESIZE = 64

	// P384ECDSA defines an ecdsa content signature on the P-384 curve
	P384ECDSA = "p384ecdsa"

	// P384ECDSABYTESIZE defines the bytes length of a P384ECDSA signature
	P384ECDSABYTESIZE = 96

	// P521ECDSA defines an ecdsa content signature on the P-521 curve
	P521ECDSA = "p521ecdsa"

	// P521ECDSABYTESIZE defines the bytes length of a P521ECDSA signature
	P521ECDSABYTESIZE = 132

	// SignaturePrefix is a string preprended to data prior to signing
	SignaturePrefix = "Content-Signature:\x00"
)

// getSignatureLen returns the size of an ECDSA signature issued by the signer,
// or -1 if the mode is unknown
//
// The signature length is double the size size of the curve field, in bytes
// (each R and S value is equal to the size of the curve field).
// If the curve field it not a multiple of 8, round to the upper multiple of 8.
func getSignatureLen(mode string) int {
	switch mode {
	case P256ECDSA:
		return P256ECDSABYTESIZE
	case P384ECDSA:
		return P384ECDSABYTESIZE
	case P521ECDSA:
		return P521ECDSABYTESIZE
	}
	return -1
}

// getSignatureHash returns the name of the hash function used by a given mode,
// or an empty string if the mode is unknown
func getSignatureHash(mode string) string {
	switch mode {
	case P256ECDSA:
		return "sha256"
	case P384ECDSA:
		return "sha384"
	case P521ECDSA:
		return "sha512"
	}
	return ""
}

// VerifyData verifies a signatures on its raw, untemplated, input using a public key
func (sig *ContentSignature) VerifyData(input []byte, pubKey *ecdsa.PublicKey) bool {
	_, hash := makeTemplatedHash(input, sig.Mode)
	return sig.VerifyHash(hash, pubKey)
}

// VerifyHash verifies a signature on its templated hash using a public key
func (sig *ContentSignature) VerifyHash(hash []byte, pubKey *ecdsa.PublicKey) bool {
	return ecdsa.Verify(pubKey, hash, sig.R, sig.S)
}

// hash returns the templated sha384 of the input data. The template adds
// the string "Content-Signature:\x00" before the input data prior to
// calculating the sha384.
//
// The name of the hash function is returned, followed by the hash bytes
func makeTemplatedHash(data []byte, curvename string) (alg string, out []byte) {
	templated := make([]byte, len(SignaturePrefix)+len(data))
	copy(templated[:len(SignaturePrefix)], []byte(SignaturePrefix))
	copy(templated[len(SignaturePrefix):], data)
	var md hash.Hash
	switch curvename {
	case P384ECDSA:
		md = sha512.New384()
		alg = "sha384"
	case P521ECDSA:
		md = sha512.New()
		alg = "sha512"
	default:
		md = sha256.New()
		alg = "sha256"
	}
	md.Write(templated)
	return alg, md.Sum(nil)
}

// Unmarshal parses a base64 url encoded content signature
// and returns it into a ContentSignature structure that can be verified.
//
// Note this function does not set the X5U value of a signature.
func Unmarshal(signature string) (sig *ContentSignature) {
	logger := log.New(&CLogger{level: 0}, "", 0)

	if len(signature) < 30 {
		logger.Println("contentsignature: signature cannot be shorter than 30 characters, got", len(signature))
		return nil
	}

	// decode the actual signature into its R and S values
	data, err := base64.RawURLEncoding.DecodeString(signature)
	if err != nil {
		logger.Println("decode string failed")
		return nil
	}

	// Use the length to determine the mode
	sig = new(ContentSignature)
	sig.Len = len(data)
	switch sig.Len {
	case P256ECDSABYTESIZE:
		sig.Mode = P256ECDSA
	case P384ECDSABYTESIZE:
		sig.Mode = P384ECDSA
	case P521ECDSABYTESIZE:
		sig.Mode = P521ECDSA
	default:
		logger.Println("contentsignature: unknown signature length", len(data))
		return nil
	}

	sig.HashName = getSignatureHash(sig.Mode)
	// parse the signature into R and S value by splitting it in the middle
	sig.R, sig.S = new(big.Int), new(big.Int)
	sig.R.SetBytes(data[:len(data)/2])
	sig.S.SetBytes(data[len(data)/2:])
	sig.Finished = true
	return sig
}

//export balrogSetLogger
func balrogSetLogger(loggerFn uintptr) {
	loggerFunc = unsafe.Pointer(loggerFn)
}

//export balrogValidateSignature
func balrogValidateSignature(publicKey string, signature string, input string) bool {
	logger := log.New(&CLogger{level: 0}, "", 0)

	cs := Unmarshal(signature)
	if cs == nil {
		logger.Println("Unmarshal failed")
		return false
	}

	block, _ := pem.Decode([]byte(publicKey))
	if block == nil {
		logger.Println("Invalid PEM public key format")
		return false
	}

	keyInterface, err := x509.ParsePKIXPublicKey(block.Bytes)
	if err != nil {
		logger.Println("Failed to parse public key DIR:", err)
		return false
	}
	pubkey := keyInterface.(*ecdsa.PublicKey)

	// verify signature on input data
	if !cs.VerifyData([]byte(input), pubkey) {
		logger.Println("Failed to verify content signature")
		return false
	}

	logger.Println("Verified")
	return true
}

func main() {}
