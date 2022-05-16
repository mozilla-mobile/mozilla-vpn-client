// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

package main

// #include <stdlib.h>
// #include <sys/types.h>
// static inline void callLogger(void *func, int level, const char *msg)
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

//export balrogSetLogger
func balrogSetLogger(loggerFn uintptr) {
	loggerFunc = unsafe.Pointer(loggerFn)
}

//export balrogValidate
func balrogValidate(x5u string, input string, signature string, rootHash string, leafCertSubject string) bool {
	logger := log.New(&CLogger{level: 0}, "", 0)
	err := contentsignature.Verify([]byte(input), []byte(x5u), signature, rootHash)
	if err != nil {
		logger.Println("Verification failed with error:", err.Error())
		return false
	}
	// Verify method does not verify the leaf cert subject, so we do it here.
	certs, _ := contentsignature.ParseChain([]byte(x5u))
	leaf := certs[0]
	if string(leaf.Subject.CommonName) != leafCertSubject {
		logger.Println("Leaf subject didn't match. Expected: ", leafCertSubject, " Got: ", leaf.Subject)
		return false
	}
	return true
}

func main() {}
