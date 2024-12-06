/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webextensionadapter.h"

#include <QFileInfo>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QTcpSocket>
#include <QWindow>

#include "connectionhealth.h"
#include "controller.h"
#include "feature/feature.h"
#include "leakdetector.h"
#include "localizer.h"
#include "logger.h"
#include "models/servercity.h"
#include "models/servercountrymodel.h"
#include "models/serverdata.h"
#include "mozillavpn.h"
#include "qmlengineholder.h"
#include "settingsholder.h"
#include "tasks/controlleraction/taskcontrolleraction.h"
#include "taskscheduler.h"
#include "webextensiontelemetry.h"

#if defined(MZ_WINDOWS)
#  include "platforms/windows/windowsutils.h"
#endif

#if defined(MZ_LINUX) && !defined(MZ_FLATPAK)
#  include <QFileInfo>
#endif

#ifdef MZ_WINDOWS
#  include "interventions/killernetwork.h"
#endif

    namespace {
  // See https://en.cppreference.com/w/cpp/utility/variant/visit
  template <class... Ts>
  struct match : Ts... {
    using Ts::operator()...;
  };
  template <class... Ts>
  match(Ts...) -> match<Ts...>;

  template <typename T>
  const char* asString(T qEnumValue) {
    const QMetaObject* meta = qt_getEnumMetaObject(qEnumValue);
    int index = meta->indexOfEnumerator(qt_getEnumName(qEnumValue));
    return meta->enumerator(index).valueToKey(qEnumValue);
  };

  Logger logger("WebExtensionAdapter");
}  // namespace

WebExtensionAdapter::WebExtensionAdapter(QObject* parent)
    : BaseAdapter(parent) {
  Q_ASSERT(parent);
  MZ_COUNT_CTOR(WebExtensionAdapter);

  MozillaVPN* vpn = MozillaVPN::instance();

  connect(vpn, &MozillaVPN::stateChanged, this,
          &WebExtensionAdapter::writeState);
  connect(vpn->controller(), &Controller::stateChanged, this,
          &WebExtensionAdapter::writeState);
  connect(vpn->connectionHealth(), &ConnectionHealth::stabilityChanged, this,
          &WebExtensionAdapter::writeState);

  m_commands = QList<RequestType>({
      RequestType{"activate",
                  [](const QJsonObject&) {
                    auto t = new TaskControllerAction(
                        TaskControllerAction::eActivateForExtension);
                    TaskScheduler::scheduleTask(t);
                    QJsonObject obj;
                    obj["ok"] = true;
                    return QJsonObject();
                  }},
      RequestType{"deactivate",
                  [](const QJsonObject&) {
                    auto t = new TaskControllerAction(
                        TaskControllerAction::eDeactivateForExtension);
                    TaskScheduler::scheduleTask(t);
                    QJsonObject obj;
                    obj["ok"] = true;
                    return QJsonObject();
                  }},
      RequestType{"servers",
                  [this](const QJsonObject&) {
                    QJsonObject servers;
                    serializeServerCountry(
                        MozillaVPN::instance()->serverCountryModel(), servers);

                    QJsonObject obj;
                    obj["servers"] = servers;
                    return obj;
                  }},
      RequestType{"focus",
                  [](const QJsonObject&) {
                    QmlEngineHolder* engine = QmlEngineHolder::instance();
                    engine->showWindow();
                    return QJsonObject{};
                  }},
      RequestType{"openAuth",
                  [](const QJsonObject&) {
                    MozillaVPN* vpn = MozillaVPN::instance();
                    if (vpn->state() != MozillaVPN::StateInitialize) {
                      return QJsonObject{};
                    }
                    vpn->authenticate();
                    return QJsonObject{};
                  }},
      RequestType{"disabled_apps",
                  [](const QJsonObject&) {
                    QJsonArray apps;
                    for (const QString& app :
                         SettingsHolder::instance()->vpnDisabledApps()) {
                      apps.append(app);
                    }

                    QJsonObject obj;
                    obj["disabled_apps"] = apps;
                    return obj;
                  }},
      RequestType{"featurelist",
                  [this](const QJsonObject&) {
                    QJsonObject obj;
                    obj["featurelist"] = serializeFeaturelist();
                    return obj;
                  }},
      RequestType{"status",
                  [this](const QJsonObject&) {
                    QJsonObject obj;
                    obj["status"] = serializeStatus();
                    return obj;
                  }},
      RequestType{"telemetry",
                  [](const QJsonObject& data) {
                    auto info = WebextensionTelemetry::fromJson(data);
                    if (info.has_value()) {
                      WebextensionTelemetry::recordTelemetry(info.value());
                    }
                    return QJsonObject{};
                  }},
      RequestType{"session_start",
                  [](const QJsonObject& data) {
                    WebextensionTelemetry::startSession();
                    return QJsonObject{};
                  }},
      RequestType{"session_stop",
                  [](const QJsonObject& data) {
                    WebextensionTelemetry::stopSession();
                    return QJsonObject{};
                  }},
      RequestType{"interventions",
                  [](const QJsonObject&) {
                    QJsonObject out;
                    QJsonArray interventions;
#ifdef MZ_WINDOWS
                    if (Intervention::KillerNetwork::systemAffected()) {
                      interventions.append(Intervention::KillerNetwork::id);
                    }
#endif
                    out["interventions"] = interventions;
                    return out;
                  }},
      RequestType{"settings",
                  [this](const QJsonObject& data) {
                    if (data["settings"].isObject()) {
                      applySettings(data["settings"].toObject());
                    }
                    return QJsonObject{{"settings", serializeSettings()}};
                  }},
  });
}

WebExtensionAdapter::~WebExtensionAdapter() {
  MZ_COUNT_DTOR(WebExtensionAdapter);
}

void WebExtensionAdapter::writeState() {
  QJsonObject obj;
  obj["status"] = serializeStatus();
  obj["t"] = "status";

  emit onOutgoingMessage(obj);
}

QJsonObject WebExtensionAdapter::serializeStatus() {
  MozillaVPN* vpn = MozillaVPN::instance();

  QJsonObject locationObj;
  locationObj["exit_country_code"] = vpn->serverData()->exitCountryCode();
  locationObj["exit_city_name"] = vpn->serverData()->exitCityName();
  locationObj["entry_country_code"] = vpn->serverData()->entryCountryCode();
  locationObj["entry_city_name"] = vpn->serverData()->entryCityName();

  QJsonObject obj;
  obj["authenticated"] = App::isUserAuthenticated();
  obj["location"] = locationObj;
  obj["version"] = Constants::versionString();
  obj["connectedSince"] =
      QString::number(vpn->controller()->connectionTimestamp());
  {
    int stateValue = vpn->state();
    if (stateValue > App::StateCustom) {
      obj["app"] = asString(static_cast<MozillaVPN::CustomState>(stateValue));
    } else {
      obj["app"] = asString(static_cast<App::State>(stateValue));
    }
  }
  obj["vpn"] = asString(vpn->controller()->state());
  obj["connectionHealth"] = asString(vpn->connectionHealth()->stability());

  return obj;
}

QJsonObject WebExtensionAdapter::serializeFeaturelist() {
  auto out = QJsonObject();
  out["webExtension"] =
      Feature::get(Feature::Feature_webExtension)->isSupported();

  // Detect the localProxy feature by checking the running services.
#if defined(MZ_LINUX) && !defined(MZ_FLATPAK)
  out["localProxy"] = QFileInfo::exists(Constants::SOCKSPROXY_UNIX_PATH);
#elif defined(MZ_WINDOWS)
  // TODO: Need to check if the service is running.
  out["localProxy"] =
      WindowsUtils::getServiceStatus(Constants::SOCKSPROXY_SERVICE_NAME);
#else
  out["localProxy"] = false;
#endif

  return out;
}

void WebExtensionAdapter::serializeServerCountry(ServerCountryModel* model,
                                                 QJsonObject& obj) {
  QJsonArray countries;

  for (const ServerCountry& country : model->countries()) {
    QJsonObject countryObj;
    countryObj["name"] = country.name();
    countryObj["code"] = country.code();

    QJsonArray cities;
    for (const QString& cityName : country.cities()) {
      const ServerCity& city = model->findCity(country.code(), cityName);
      if (!city.initialized()) {
        continue;
      }
      QJsonObject cityObj;
      cityObj["name"] = city.name();
      cityObj["code"] = city.code();
      cityObj["latitude"] = city.latitude();
      cityObj["longitude"] = city.longitude();

      QJsonArray servers;
      for (const QString& pubkey : city.servers()) {
        const Server& server = model->server(pubkey);
        if (!server.initialized()) {
          continue;
        }

        QJsonObject serverObj;
        serverObj["hostname"] = server.hostname();
        serverObj["ipv4_gateway"] = server.ipv4Gateway();
        serverObj["ipv6_gateway"] = server.ipv6Gateway();
        serverObj["weight"] = (double)server.weight();

        const QString& socksName = server.socksName();
        if (!socksName.isEmpty()) {
          serverObj["socksName"] = socksName;
        }

        uint32_t multihopPort = server.multihopPort();
        if (multihopPort) {
          serverObj["multihopPort"] = (double)multihopPort;
        }

        servers.append(serverObj);
      }

      cityObj["servers"] = servers;
      cities.append(cityObj);
    }

    countryObj["cities"] = cities;
    countries.append(countryObj);
  }

  obj["countries"] = countries;
}

QJsonObject WebExtensionAdapter::serializeSettings() {
  auto const settings = SettingsHolder::instance();
  return {{"extensionTelemetryEnabled", settings->extensionTelemetryEnabled()}};
}

void WebExtensionAdapter::applySettings(const QJsonObject& data) {
  auto const settings = SettingsHolder::instance();

  auto enabled = data["extensionTelemetryEnabled"];
  if (enabled.isBool()) {
    settings->setExtensionTelemetryEnabled(enabled.toBool());
  }
}
