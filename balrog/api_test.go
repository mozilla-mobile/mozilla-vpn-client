// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

package main

import (
	"testing"
)

func TestGetSignatureLen(t *testing.T) {
	TESTCASES := []struct {
		mode   string
		length int
	}{
		{"p256ecdsa", 64},
		{"p384ecdsa", 96},
		{"p521ecdsa", 132},
		{"unsupported", -1},
	}
	for _, testcase := range TESTCASES {
		got := getSignatureLen(testcase.mode)
		if got != testcase.length {
			t.Errorf("getSignatureLen(%#v) = %d, expected %d.", testcase.mode, got, testcase.length)
		}
	}
}

func TestGetSignatureHash(t *testing.T) {
	TESTCASES := []struct {
		mode string
		name string
	}{
		{"p256ecdsa", "sha256"},
		{"p384ecdsa", "sha384"},
		{"p521ecdsa", "sha512"},
		{"unsupported", ""},
	}
	for _, testcase := range TESTCASES {
		got := getSignatureHash(testcase.mode)
		if got != testcase.name {
			t.Errorf("getSignatureHash(%#v) = %s, expected %s.", testcase.mode, got, testcase.name)
		}
	}
}
