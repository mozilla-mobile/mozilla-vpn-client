/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QByteArray>

class QIODevice;
class NetworkRequest;

class WasmNetworkRequest final {
 public:
  static void deleteResource(NetworkRequest* request);
  static void getResource(NetworkRequest* request);
  static void postResporce(NetworkRequest* request, const QByteArray& body);
  static void postResporceIODevice(NetworkRequest* request, QIODevice* device);

 private:
  static void processResponse(const QByteArray& data, NetworkRequest* request);
};
