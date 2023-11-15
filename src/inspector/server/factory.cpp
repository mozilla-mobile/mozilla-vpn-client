/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "factory.h"
#include "../inspector.h"

#include <QObject>

#ifndef MZ_WASM
	#include "websocketserver.h"
#else
	// Todo: do wasm
#endif  // !MZ_WASM
namespace InspectorServer {

QObject* Factory::create(Inspector* parent) {
#ifndef MZ_WASM
  return new InspectorWebSocketServer(parent);
#else
  return nullptr;
  // Todo: do wasm
#endif  // !MZ_WASM
}

}

