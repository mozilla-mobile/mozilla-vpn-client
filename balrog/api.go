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
	"errors"
	"log"
	"unsafe"

	"github.com/mozilla-services/autograph/verifier/contentsignature"
)

var loggerFunc unsafe.Pointer

type CLogger struct {
	level C.int
}

func (l *CLogger) Write(p []byte) (int, error) {
	if uintptr(loggerFunc) == 0 {
		return 0, errors.New("no logger initialized")
	}
	message := C.CString(string(p))
	C.callLogger(loggerFunc, l.level, message)
	C.free(unsafe.Pointer(message))
	return len(p), nil
}

//
// NEW METHOD SIGNATURE - balrogValidate(x5uDataGo, updateDataGo, signatureGo, certSubjectGo);
// - parse x5uData
// - validate chain
// - validate root (with root cert fingerprint?)
// - validate leaf (with certSubject)
// - get publickey from leaf
// - validate updateData (aka input) with publickey and signature
//
//
func balrogValidate(x5u string, input string, signature string, certSubject string) bool {
	rootHash := "rootHash"
	certChain := "certChain"
	err := contentsignature.Verify([]byte(input), []byte(certChain), signature, rootHash)
	if err != nil {
		logger := log.New(&CLogger{level: 0}, "", 0)
		logger.Println("Verification failed with error:", err.Error())
		return false
	}
	return true
}

func main() {}
