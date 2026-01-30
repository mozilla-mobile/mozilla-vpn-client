/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "uploader.h"

#include <QBuffer>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

int vpn_ping_upload(const vpn_ping_payload* payload) {
#ifdef __wasm__
  return 200;
#else
  qWarning() << "Glean upload to:" << payload->url;

  // Create the HTTP request.
  QUrl url(payload->url);
  QNetworkRequest req(url);
  req.setTransferTimeout(30000);
  for (int i = 0; payload->headers[i].name; i++) {
    const struct vpn_ping_header* hdr = &payload->headers[i];
    req.setRawHeader(QByteArray(hdr->name), QByteArray(hdr->value));
  }

  QBuffer body;
  body.setData((const char *)payload->body, payload->length);

  // Start the request.
  QNetworkAccessManager manager;
  QNetworkReply* reply = manager.post(req, &body);
  QObject::connect(reply, &QNetworkReply::finished, reply,
                   &QObject::deleteLater);

  // Run the event loop until the HTTP reply is finished.
  QEventLoop loop;
  QObject::connect(reply, &QNetworkReply::finished, &loop, [&](){
    QVariant qv = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!qv.isValid()) {
      loop.exit(-1);
    }
    loop.exit(qv.toInt());
  });
  return loop.exec();
#endif
}
