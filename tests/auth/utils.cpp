/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utils.h"
#include "../../src/networkrequest.h"
#include "../../src/tasks/function/taskfunction.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

class EventLoop final : public QEventLoop {
 public:
  void exec() {
    QTimer timer;
    connect(&timer, &QTimer::timeout, [&]() {
      qDebug() << "TIMEOUT!";
      exit();
    });
    timer.setSingleShot(true);
    timer.start(60000 /* 60 seconds */);

    QEventLoop::exec();
  }
};

// static
QString TestUtils::fetchSessionCode(const QString& account) {
  return fetchCode(account, "x-verify-short-code");
}

// static
QString TestUtils::fetchUnblockCode(const QString& account) {
  return fetchCode(account, "x-unblock-code");
}

// static
QString TestUtils::fetchCode(const QString& account, const QString& code) {
  while (true) {
    QString url = "http://restmail.net/mail/";
    url.append(account);

    // In theory, network requests should be executed by tasks, but for this
    // test we do an hack.
    TaskFunction dummyTask([] {});

    NetworkRequest* nr = NetworkRequest::createForGetUrl(&dummyTask, url);

    QByteArray jsonData;
    EventLoop loop;
    QObject::connect(nr, &NetworkRequest::requestFailed,
                     [&](QNetworkReply::NetworkError, const QByteArray&) {
                       qDebug() << "Failed to fetch the restmail.net content";
                       loop.exit();
                     });
    QObject::connect(nr, &NetworkRequest::requestCompleted,
                     [&](const QByteArray& data) {
                       jsonData = data;
                       loop.exit();
                     });
    loop.exec();

    QJsonDocument doc(QJsonDocument::fromJson(jsonData));
    QJsonArray array = doc.array();
    if (!array.isEmpty()) {
      QJsonObject obj = array.last().toObject();
      QJsonObject headers = obj["headers"].toObject();
      if (headers.contains(code)) {
        return headers[code].toString();
      }
    }

    qDebug() << "Email not received yet";

    QTimer timer;
    QObject::connect(&timer, &QTimer::timeout, [&]() { loop.exit(); });
    timer.setSingleShot(true);
    timer.start(2000 /* 2 seconds */);
    loop.exec();
  }
}
