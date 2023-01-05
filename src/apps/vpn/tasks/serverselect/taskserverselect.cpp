/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskserverselect.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QtMath>

#include "errorhandler.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskServerSelect");
}

TaskServerSelect::TaskServerSelect(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskServerSelect"),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MZ_COUNT_CTOR(TaskServerSelect);
}

TaskServerSelect::~TaskServerSelect() { MZ_COUNT_DTOR(TaskServerSelect); }

void TaskServerSelect::run() {
  NetworkRequest* request = NetworkRequest::createForIpInfo(this);

  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            logger.error() << "Failed to retrieve client GeoIP";
            REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            QStringList choice = processData(data);
            Q_ASSERT(choice.length() >= 2);

            ServerData* sd = MozillaVPN::instance()->currentServer();
            sd->update(choice[0], choice[1]);
            Q_ASSERT(sd->hasServerData());

            emit completed();
          });
}

QStringList TaskServerSelect::processData(const QByteArray& data) {
  ServerCountryModel* scm = MozillaVPN::instance()->serverCountryModel();

  QJsonDocument json = QJsonDocument::fromJson(data);
  if (!json.isObject()) {
    logger.error() << "Invalid data returned from the GeoIP lookup";
    return scm->pickRandom();
  }

  QJsonObject obj = json.object();
  QStringList latlong = obj.value("lat_long").toString().split(',');
  if (latlong.count() != 2) {
    logger.info() << "No GeoIP data returned in ipinfo lookup";
    return scm->pickRandom();
  }
  double latitude = latlong[0].toDouble();
  double longitude = latlong[1].toDouble();
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
  for (const ServerCountry& country : scm->countries()) {
    for (const ServerCity& city : country.cities()) {
      double citySin = qSin(city.latitude() * M_PI / 180.0);
      double cityCos = qCos(city.latitude() * M_PI / 180.0);
      double diffCos = qCos((city.longitude() - longitude) * M_PI / 180.0);
      double distance =
          qAcos(clientSin * citySin + clientCos * cityCos * diffCos);

      if (distance < bestDistance) {
        bestCountry = country.code();
        bestCity = city.name();
        bestDistance = distance;
      }
    }
  }

  if (bestCountry.isEmpty() || bestCity.isEmpty()) {
    return scm->pickRandom();
  }
  return QStringList({bestCountry, bestCity});
}
