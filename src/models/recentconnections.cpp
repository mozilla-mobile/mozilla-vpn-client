/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "recentconnections.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "serverdata.h"
#include "serveri18n.h"
#include "settingsholder.h"

namespace {
Logger logger("RecentConnections");
}

constexpr const char* ENTER_CITY_NAME = "enter_city_name";
constexpr const char* ENTER_COUNTRY_CODE = "enter_country_code";
constexpr const char* EXIT_CITY_NAME = "exit_city_name";
constexpr const char* EXIT_COUNTRY_CODE = "exit_country_code";

// static
RecentConnections* RecentConnections::instance() {
  static RecentConnections* s_instance = nullptr;
  if (!s_instance) {
    s_instance = new RecentConnections(qApp);
  }
  return s_instance;
}

RecentConnections::RecentConnections(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(RecentConnections);
}

RecentConnections::~RecentConnections() { MZ_COUNT_DTOR(RecentConnections); }

void RecentConnections::initialize() {
  connect(MozillaVPN::instance()->serverData(), &ServerData::changed, this,
          &RecentConnections::serverChanged);

  m_singleHopModel.clear();
  m_multiHopModel.clear();

  if (!migrate()) {
    readSettings();
  }
}

bool RecentConnections::migrate() {
  SettingsHolder* settingsHolder = SettingsHolder::instance();
  Q_ASSERT(settingsHolder);

  // We need to migrate the recent-connection settings stored as a QStringList
  if (!settingsHolder->hasRecentConnectionsDeprecated()) {
    return false;
  }

  QStringList rcList = settingsHolder->recentConnectionsDeprecated();
  for (const QString& rc : rcList) {
    QStringList parts = rc.split(" -> ");
    QStringList countryCityPartA = parts[0].split(", ");
    if (countryCityPartA.length() != 2) {
      logger.debug() << "Invalid deprecated entry:" << parts[0];
      continue;
    }

    if (parts.length() == 1) {
      RecentConnection recentConnection(countryCityPartA[0],
                                        countryCityPartA[1]);
      m_singleHopModel.add(recentConnection, RecentConnectionModel::Append);
      continue;
    }

    QStringList countryCityPartB = parts[1].split(", ");
    if (countryCityPartB.length() != 2) {
      logger.debug() << "Invalid deprecated entry:" << parts[1];
      continue;
    }

    RecentConnection recentConnection(countryCityPartA[0], countryCityPartA[1],
                                      countryCityPartB[0], countryCityPartB[1]);
    m_multiHopModel.add(recentConnection, RecentConnectionModel::Append);
  }

  settingsHolder->removeRecentConnectionsDeprecated();
  writeSettings();
  return true;
}

void RecentConnections::serverChanged() {
  logger.info() << "server changed";
  ServerData* sd = MozillaVPN::instance()->serverData();

  RecentConnection newRecentConnection{
      sd->entryCityName(), sd->entryCountryCode(), sd->exitCityName(),
      sd->exitCountryCode()};

  bool ok = false;
  if (newRecentConnection.isMultiHop()) {
    ok = m_multiHopModel.add(newRecentConnection,
                             RecentConnectionModel::Prepend);
  } else {
    ok = m_singleHopModel.add(newRecentConnection,
                              RecentConnectionModel::Prepend);
  }

  if (ok) {
    writeSettings();
  }
}

void RecentConnections::readSettings() {
  QJsonDocument json =
      QJsonDocument::fromJson(SettingsHolder::instance()->recentConnections());
  if (!json.isArray()) {
    return;
  }

  QJsonArray array = json.array();
  for (const QJsonValue& value : array) {
    QJsonObject obj = value.toObject();

    RecentConnection rc(
        obj[ENTER_CITY_NAME].toString(), obj[ENTER_COUNTRY_CODE].toString(),
        obj[EXIT_CITY_NAME].toString(), obj[EXIT_COUNTRY_CODE].toString());

    if (rc.isMultiHop()) {
      m_multiHopModel.add(rc, RecentConnectionModel::Append);
    } else {
      m_singleHopModel.add(rc, RecentConnectionModel::Append);
    }
  }
}

void RecentConnections::writeSettings() {
  QJsonArray array;
  m_singleHopModel.serialize(array);
  m_multiHopModel.serialize(array);

  SettingsHolder::instance()->setRecentConnections(
      QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QHash<int, QByteArray> RecentConnectionModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[ExitCountryCodeRole] = "exitCountryCode";
  roles[ExitCityNameRole] = "exitCityName";
  roles[LocalizedExitCityNameRole] = "localizedExitCityName";
  roles[IsMultiHopRole] = "isMultiHop";
  roles[EntryCountryCodeRole] = "entryCountryCode";
  roles[EntryCityNameRole] = "entryCityName";
  roles[LocalizedEntryCityNameRole] = "localizedEntryCityName";
  return roles;
}

QVariant RecentConnectionModel::data(const QModelIndex& index, int role) const {
  // We want to skip the first element because it's the current connection.
  int id = index.row() + 1;

  if (!index.isValid() || id >= m_list.length()) {
    return QVariant();
  }

  switch (role) {
    case ExitCountryCodeRole:
      return QVariant(m_list.at(id).m_exitCountryCode);

    case ExitCityNameRole:
      return QVariant(m_list.at(id).m_exitCityName);

    case LocalizedExitCityNameRole: {
      const RecentConnection& rc = m_list.at(id);
      return QVariant(ServerI18N::instance()->translateCityName(
          rc.m_exitCountryCode, rc.m_exitCityName));
    }

    case IsMultiHopRole: {
      const RecentConnection& rc = m_list.at(id);
      return QVariant(!rc.m_entryCountryCode.isEmpty() &&
                      !rc.m_entryCityName.isEmpty());
    }

    case EntryCountryCodeRole:
      return QVariant(m_list.at(id).m_entryCountryCode);

    case EntryCityNameRole:
      return QVariant(m_list.at(id).m_entryCityName);

    case LocalizedEntryCityNameRole: {
      const RecentConnection& rc = m_list.at(id);
      return QVariant(ServerI18N::instance()->translateCityName(
          rc.m_entryCountryCode, rc.m_entryCityName));
    }

    default:
      return QVariant();
  }
}

int RecentConnectionModel::rowCount(const QModelIndex&) const {
  int size = static_cast<int>(m_list.length());
  return size > 1 ? size - 1 : 0;
}

void RecentConnectionModel::clear() {
  beginResetModel();
  m_list.clear();
  endResetModel();
  emit modelChanged();
}

bool RecentConnectionModel::add(RecentConnection& newRecentConnection,
                                AddType addType) {
  qsizetype index = m_list.indexOf(newRecentConnection);
  if (index == 0) {
    // This is already the most-recent connection.
    return false;
  }

  beginResetModel();

  if (index > 0) {
    m_list.removeAt(index);
  }

  while (m_list.count() > Constants::RECENT_CONNECTIONS_MAX_COUNT) {
    m_list.removeLast();
  }

  switch (addType) {
    case Prepend:
      m_list.prepend(newRecentConnection);
      break;

    case Append:
      m_list.append(newRecentConnection);
      break;
  }

  endResetModel();
  emit modelChanged();

  return true;
}

void RecentConnectionModel::serialize(QJsonArray& array) {
  for (const RecentConnection& rc : m_list) {
    QJsonObject obj;
    obj[EXIT_COUNTRY_CODE] = rc.m_exitCountryCode;
    obj[EXIT_CITY_NAME] = rc.m_exitCityName;
    obj[ENTER_COUNTRY_CODE] = rc.m_entryCountryCode;
    obj[ENTER_CITY_NAME] = rc.m_entryCityName;
    array.append(obj);
  }
}
