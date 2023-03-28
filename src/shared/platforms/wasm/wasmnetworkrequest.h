/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QByteArray>

class QIODevice;
class NetworkRequest;

class WasmNetworkRequest final {
 public:
  static bool deleteResource(NetworkRequest* request);
  static bool getResource(NetworkRequest* request);
  static bool postResource(NetworkRequest* request, const QByteArray& body);
  static bool postResourceIODevice(NetworkRequest* request, QIODevice* device);

 private:
  static void processResponse(const QByteArray& data, NetworkRequest* request);
};
