/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ERROR_TYPE_H
#define ERROR_TYPE_H

#if not(defined(__wasm__) || defined(BUILD_QMAKE))
#  include "qtglean.h"
#else
enum ErrorType {};
#endif

#endif  // ERROR_TYPE_H
