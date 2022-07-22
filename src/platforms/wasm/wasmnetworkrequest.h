/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <QByteArray>

class NetworkRequest;

class WasmNetworkRequest final {
 public:
  static void deleteRequest(NetworkRequest* request);
  static void getRequest(NetworkRequest* request);
  static void postRequest(NetworkRequest* request, const QByteArray& body);

 private:
  static void processResponse(const QByteArray& data, NetworkRequest* request);
};
