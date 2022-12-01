/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RECENTCONNECTIONS_H
#define RECENTCONNECTIONS_H

#include <QAbstractListModel>

class QJsonArray;

struct RecentConnection {
  QString m_entryCityName;
  QString m_entryCountryCode;
  QString m_exitCityName;
  QString m_exitCountryCode;

  RecentConnection(const QString& exitCityName, const QString& exitCountryCode)
      : m_exitCityName(exitCityName), m_exitCountryCode(exitCountryCode) {}

  RecentConnection(const QString& entryCityName,
                   const QString& entryCountryCode, const QString& exitCityName,
                   const QString& exitCountryCode)
      : m_entryCityName(entryCityName),
        m_entryCountryCode(entryCountryCode),
        m_exitCityName(exitCityName),
        m_exitCountryCode(exitCountryCode) {}

  bool isMultiHop() const {
    return !m_entryCityName.isEmpty() && !m_entryCountryCode.isEmpty();
  }

  // Used by QList::indexOf()
  bool operator==(const RecentConnection& other) const {
    return m_entryCityName == other.m_entryCityName &&
           m_entryCountryCode == other.m_entryCountryCode &&
           m_exitCityName == other.m_exitCityName &&
           m_exitCountryCode == other.m_exitCountryCode;
  }
};

class RecentConnectionModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(RecentConnectionModel)

  Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY modelChanged)

 public:
  RecentConnectionModel() = default;
  ~RecentConnectionModel() = default;

  enum RecentCOnnectionRoles {
    ExitCountryCodeRole = Qt::UserRole + 1,
    ExitCityNameRole,
    LocalizedExitCityNameRole,
    IsMultiHopRole,
    EntryCountryCodeRole,
    EntryCityNameRole,
    LocalizedEntryCityNameRole,
  };

  enum AddType {
    Prepend,
    Append,
  };

  bool isEmpty() const { return m_list.length() <= 1; }

  void clear();

  bool add(RecentConnection& newRecentConnection, AddType addType);

  void serialize(QJsonArray& array);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void modelChanged();

 private:
  QList<RecentConnection> m_list;
};

class RecentConnections final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(RecentConnections)

  Q_PROPERTY(
      const RecentConnectionModel* singleHopModel READ singleHopModel CONSTANT)
  Q_PROPERTY(
      const RecentConnectionModel* multiHopModel READ multiHopModel CONSTANT)

 public:
  static RecentConnections* instance();
  ~RecentConnections();

  void initialize();

  const RecentConnectionModel* singleHopModel() const {
    return &m_singleHopModel;
  }
  const RecentConnectionModel* multiHopModel() const {
    return &m_multiHopModel;
  }

 private:
  explicit RecentConnections(QObject* parent);

  void serverChanged();

  void readSettings();
  void writeSettings();

  bool migrate();

 private:
  RecentConnectionModel m_singleHopModel;
  RecentConnectionModel m_multiHopModel;
};

#endif  // RECENTCONNECTIONS_H
