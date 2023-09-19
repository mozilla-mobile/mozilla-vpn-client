/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "inspectorserver.h"
#include "inspector.h"

#ifndef MZ_WASM
	#include "inspectorwebsocketserver.h"
#else
	// Todo: do wasm
#endif  // !MZ_WASM



QObject* InspectorServerFactory::create(Inspector* parent) {
#ifndef MZ_WASM
  return new InspectorWebSocketServer(parent);
#else
	return nullptr;
	// Todo: do wasm
#endif  // !MZ_WASM


}
