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

	"github.com/mozilla-services/autograph/signer/contentsignaturepki"
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

//export balrogValidateSignature
func balrogValidateSignature(x5u string, signature string, input string) bool {
	logger := log.New(&CLogger{level: 0}, "", 0)
	err := contentsignaturepki.Verify(x5u, signature, []byte(input))
	if err != nil {
		logger.Println("Signature verification failed with error:", err.Error())
		return false
	}
	logger.Println("Signature verified")
	return true
}

func main() {}
