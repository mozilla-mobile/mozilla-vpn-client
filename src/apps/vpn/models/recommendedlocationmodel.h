/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RECOMMENDEDLOCATIONMODEL_H
#define RECOMMENDEDLOCATIONMODEL_H

#include <QAbstractListModel>
#include <QPointer>
#include <QSet>
#include <QTimer>

#include "servercity.h"

class RecommendedLocationModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(RecommendedLocationModel)

 public:
  enum RecommendedLocationRoles {
    CityRole = Qt::UserRole + 1,
  };

  static RecommendedLocationModel* instance();
  ~RecommendedLocationModel();

  void initialize();

  Q_INVOKABLE static QList<const ServerCity*> recommendedLocationsRaw(
      unsigned int maxResults);

  static QList<QPointer<ServerCity>> recommendedLocations(
      unsigned int maxResults);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override {
    return static_cast<int>(m_recommendedCities.length());
  }

  QVariant data(const QModelIndex& index, int role) const override;

 private:
  explicit RecommendedLocationModel(QObject* parent);

  void maybeRefreshModel();
  void refreshModel();

 private:
  QList<QPointer<ServerCity>> m_recommendedCities;
  QTimer m_timer;
};

#endif  // RECOMMENDEDLOCATIONMODEL_H
