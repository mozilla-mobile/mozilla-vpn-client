/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskserverselect.h"
#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

#include <QtMath>
#include <QJsonDocument>
#include <QJsonObject>

// !!!DO NOT MERGE!!!
// FIXME: Use of this API is permitted for project use only, we would need
// to purchase an API key for commercial usage, or we just add lat/long client
// coordinates to the guardian ipinfo API. But hopefully this is fine as a
// proof-of-concept.
constexpr const char* SERVER_GUESS_GEOIP_URL = "https://json.geoiplookup.io/";

namespace {
Logger logger(LOG_MAIN, "TaskServerSelect");
}

TaskServerSelect::TaskServerSelect() : Task("TaskServerSelect") {
  MVPN_COUNT_CTOR(TaskServerSelect);
}

TaskServerSelect::~TaskServerSelect() { MVPN_COUNT_DTOR(TaskServerSelect); }

void TaskServerSelect::run() {
  NetworkRequest* request =
      NetworkRequest::createForGetUrl(this, SERVER_GUESS_GEOIP_URL, 200);

  connect(
      request, &NetworkRequest::requestFailed, this,
      [this](QNetworkReply::NetworkError error, const QByteArray&) {
        logger.error() << "Failed to retrieve client GeoIP";
        MozillaVPN::instance()->errorHandle(ErrorHandler::toErrorType(error));
        emit completed();
      });

  connect(request, &NetworkRequest::requestCompleted, this,
      [this](const QByteArray& data){
        processData(data);
        emit completed();
      });
}

void TaskServerSelect::processData(const QByteArray& data) {
  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.error() << "Invalid data returned from the GeoIP lookup";
    return;
  }
  QJsonObject obj = json.object();
  double latitude = obj.value("latitude").toDouble();
  double longitude = obj.value("longitude").toDouble();
  double clientSin = qSin(latitude * M_PI / 180.0);
  double clientCos = qCos(latitude * M_PI / 180.0);
  logger.debug() << "Client located at"
                 << logger.sensitive(QString::number(latitude))
                 << logger.sensitive(QString::number(longitude));

  // We rank cities using the distance between two points on a great circle,
  // which is given by:
  //    d = acos(sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(long1-long2))
  QString bestCountry;
  QString bestCity;
  double bestDistance = M_2_PI; 
  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();
  for (const ServerCountry& country : scm->countries()) {
    for (const ServerCity& city : country.cities()) {
      double citySin = qSin(city.latitude() * M_PI / 180.0);
      double cityCos = qCos(city.latitude() * M_PI / 180.0);
      double diffCos = qCos((city.longitude() - longitude) * M_PI / 180.0);
      double distance = qAcos(clientSin*citySin + clientCos*cityCos*diffCos);

      if (distance < bestDistance) {
        bestCountry = country.code();
        bestCity = city.name();
        bestDistance = distance;
      }
    }
  }

  ServerData* sd = MozillaVPN::instance()->currentServer();
  if (!bestCountry.isEmpty() && !bestCity.isEmpty()) {
    sd->update(bestCountry, bestCity);
    Q_ASSERT(sd->initialized());
    sd->writeSettings();
  }
}
