/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmnetworkrequest.h"
#include "logger.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>

namespace {
Logger logger(LOG_MAIN, "WasmNetworkRequest");

QMap<int, NetworkRequest*> s_networkRequestMap;
int m_networkRequestId = 0;

void processNetworkResponse(int id, const QByteArray& data) {
  if (!s_networkRequestMap.contains(id)) {
    logger.debug() << "Network request with ID" << id << "already dismissed";
    return;
  }

  NetworkRequest* request = s_networkRequestMap.take(id);
  Q_ASSERT(request);

  QJsonDocument doc = QJsonDocument::fromJson(data);
  QJsonObject obj = doc.object();

  int status = obj["status"].toInt();
  QByteArray body = QByteArray::fromBase64(obj["body"].toString().toUtf8());
  QNetworkReply::NetworkError error = QNetworkReply::NoError;

  if (status >= 200 && status <= 299) {
    request->processData(error, QString(), status, body);
    return;
  }

  if (status == 401) {
    request->processData(QNetworkReply::AuthenticationRequiredError,
                         QString("Unauthorized"), status, body);
    return;
  }

  if (status == 403) {
    request->processData(QNetworkReply::ContentAccessDenied,
                         QString("Access denied"), status, body);
    return;
  }

  if (status == 404) {
    request->processData(QNetworkReply::ContentNotFoundError,
                         QString("Not found"), status, body);
    return;
  }

  request->processData(QNetworkReply::ConnectionRefusedError,
                       QString("Invalid JS response"), status, body);
}

int processNetworkRequest(NetworkRequest* request) {
  int id = ++m_networkRequestId;
  s_networkRequestMap.insert(id, request);

  QObject::connect(request, &QObject::destroyed,
                   [id]() { s_networkRequestMap.remove(id); });

  return id;
}

}  // namespace

EMSCRIPTEN_KEEPALIVE void mvpnNetworkResponse(emscripten::val id,
                                              emscripten::val input) {
  std::string str = input.as<std::string>();
  processNetworkResponse(id.as<int>(), QByteArray(str.c_str(), str.length()));
}

EMSCRIPTEN_BINDINGS(MozillaVPNResponse) {
  emscripten::function("mvpnNetworkResponse", &mvpnNetworkResponse);
}

EM_JS(void, call_mvpnNetworkRequest,
      (int id, const char* method, const char* url, const char* body,
       int bodyLength),
      {
        try {
          mvpnNetworkRequest(id, UTF8ToString(method), UTF8ToString(url),
                             UTF8ToString(body, bodyLength));
        } catch (e) {
          console.log("Failed to process the network request in JS", e);
        }
      });

// static
void WasmNetworkRequest::deleteRequest(NetworkRequest* request) {
  logger.debug() << "Delete request to JS";

  call_mvpnNetworkRequest(processNetworkRequest(request), "DELETE",
                          request->url().toString().toUtf8().data(), "", 0);
}

// static
void WasmNetworkRequest::getRequest(NetworkRequest* request) {
  logger.debug() << "Get request to JS";

  call_mvpnNetworkRequest(processNetworkRequest(request), "GET",
                          request->url().toString().toUtf8().data(), "", 0);
}

// static
void WasmNetworkRequest::postRequest(NetworkRequest* request,
                                     const QByteArray& body) {
  logger.debug() << "Post request to JS";

  call_mvpnNetworkRequest(processNetworkRequest(request), "POST",
                          request->url().toString().toUtf8().data(),
                          body.data(), body.length());
}
