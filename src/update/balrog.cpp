/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "balrog.h"
#include "leakdetector.h"
#include "logger.h"
#include "networkrequest.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

constexpr const char* BALROG_URL =
    "https://aus5.mozilla.org/json/1/FirefoxVPN/%1/%2/release/update.json";

constexpr const char* BALROG_WINDOWS_UA64 = "WINNT_x86_64";
constexpr const char* BALROG_WINDOWS_UA32 = "WINNT_x86_32";

namespace {
Logger logger(LOG_NETWORKING, "Balrog");
}

Balrog::Balrog(QObject* parent) : Updater(parent) {
  MVPN_COUNT_CTOR(Balrog);
  logger.log() << "Balrog created";
}

Balrog::~Balrog() {
  MVPN_COUNT_DTOR(Balrog);
  logger.log() << "Balrog released";
}

// static
QString Balrog::userAgent() {
  static bool h =
      QSysInfo::currentCpuArchitecture().contains(QLatin1String("64"));
  return h ? BALROG_WINDOWS_UA64 : BALROG_WINDOWS_UA32;
}

void Balrog::start() {
  QString url = QString(BALROG_URL).arg(APP_VERSION).arg(userAgent());
  logger.log() << "URL:" << url;

  NetworkRequest* request = NetworkRequest::createForUrl(this, url);

  connect(request, &NetworkRequest::requestFailed,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.log() << "Versions request failed" << error;
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this](QNetworkReply*, const QByteArray& data) {
            logger.log() << "Account request completed";

            if (!processData(data)) {
              logger.log() << "Ignore failure.";
            }
          });

  connect(request, &QObject::destroyed, this, &QObject::deleteLater);
}

bool Balrog::processData(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.log() << "A valid JSON object expected";
    return false;
  }

  QJsonObject obj = json.object();

  if (obj.value("required").toBool()) {
    emit updateRequired();
    return true;
  }

  emit updateRecommended();
  return true;
}
