// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

package main

import (
	"math/big"
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

func TestUnmarshalShortLen(t *testing.T) {
	cs := Unmarshal("")
	// Logger error message should be: "contentsignature: signature cannot be shorter than 30 characters, got 0"
	// Suggest adding back error return and logging in main method.
	if cs != nil {
		t.Fatalf("expected to fail with 'signature cannot be shorter than 30 characters', but got a content signature.")
	}
}

func TestUnmarshalBadBase64(t *testing.T) {
	cs := Unmarshal("gZimwQAsuCj_JcgxrIjw1wzON8WYN9YKp3I5I9NmOgnGLOJJwHDxjOA2QEnzN7bXBGWFgn8HJ7fGRYxBy1SHiDMiF8VX7V49KkanO9MO-RRN1AyC9xmghuEcF4ndhQaIgZimwQAsuCj_JcgxrIjw1wzON8WYN9YKp3I5I9NmOgnGLOJJwHDxjOA2QEnzN7bXBGWFgn8HJ7fGRYxBy1SHiDMiF8VX7V49KkanO9MO-RRN1AyC9xmghuEcF4ndhQaI")
	// Logger error message should be: "contentsignature: unknown signature length 192"
	if cs != nil {
		t.Fatalf("expected to fail with 'unknown signature length', but got a content signature.")
	}
}

func TestUnmarshalGood(t *testing.T) {
	R1, _ := new(big.Int).SetString("22266518595888943254548287837336613610534337769189411655374081483350242789601254602883214629839070631871429044359750", 0)
	S1, _ := new(big.Int).SetString("32670981709832019440771636400609295498964459146778320656943912007443814089467941690326788177830169529334908958485331", 0)
	R2, _ := new(big.Int).SetString("51825061746056164233889388697418451568781711721650778870854589550953507592648", 0)
	S2, _ := new(big.Int).SetString("99818375319858346063353010573340696332409992506273214579512479209801834822360", 0)
	TESTCASES := []struct {
		signature string
		cs        ContentSignature
	}{
		{
			signature: "kKsiMiCJjFjaQHcPGXdvJKyL6OHXtcpuQ1pSpigtIfoVj4e1ttaC_QSll_kwoFJG1ESEzi9i35FmwdMiJ-3qWgCaMxV7a-7SCGbt384FBZcIlIVZjaCm0Ejn2cTMcytT",
			cs: ContentSignature{
				R:        R1,
				S:        S1,
				Mode:     "p384ecdsa",
				HashName: "sha384",
				X5U:      "",
				ID:       "",
				Len:      96,
				Finished: true,
			},
		},
		{
			signature: "cpPyCeHcZKrP-n9BZZojCHvOOEGeJvKTHTX1X44oGcjcrzLE4WSRg_sCHZnJ9FO6-gmUSa4uipzpObKGZR_62A",
			cs: ContentSignature{
				R:        R2,
				S:        S2,
				Mode:     "p256ecdsa",
				HashName: "sha256",
				X5U:      "",
				ID:       "",
				Len:      64,
				Finished: true,
			},
		},
	}
	for _, testcase := range TESTCASES {
		expected := testcase.cs
		got := Unmarshal(testcase.signature)
		if got.R.Cmp(expected.R) != 0 {
			t.Fatalf("Unmarshal did not set valid R. \n\t Got:\t\t%s \n\t Expected:\t%s", got.R, expected.R)
		}
		if got.S.Cmp(expected.S) != 0 {
			t.Fatalf("Unmarshal did not set valid S. \n\t Got:\t\t%s \n\t Expected:\t%s", got.S, expected.S)
		}
		if got.HashName != expected.HashName {
			t.Fatalf("Unmarshal did not set valid Hashname. \n\t Got:\t\t%s \n\t Expected:\t%s", got.HashName, expected.HashName)
		}
		if got.Mode != expected.Mode {
			t.Fatalf("Unmarshal did not set valid Mode. \n\t Got:\t\t%s \n\t Expected:\t%s", got.Mode, expected.Mode)
		}
		if got.Len != expected.Len {
			t.Fatalf("Unmarshal did not set valid Len. \n\t Got:\t\t%d \n\t Expected:\t%d", got.Len, expected.Len)
		}
		if got.Finished != expected.Finished {
			t.Fatalf("Unmarshal did not set Finished to true on valid signature. \n\t Got:\t\t%v \n\t Expected:\t%v", got.Finished, expected.Finished)
		}
	}
}
