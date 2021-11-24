/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATURELIST_H
#define FEATURELIST_H

#include <QObject>
#include <QAbstractListModel>

class Feature;

class FeatureList final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FeatureList)

 private:
  FeatureList() = default;

 public:
  enum ModelRoles {
    FeatureRole = Qt::UserRole + 1,
  };

  static FeatureList& instance();

  void initialize();

  void updateFeatureList(const QByteArray& data);

  // QAbstractListModel methods
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  Q_INVOKABLE void devModeFlipFeatureFlag(const QString& feature);
  Q_INVOKABLE QObject* get(const QString& feature);

 private:
  QList<Feature*> m_featurelist;
};

#endif  // FEATURELIST_H
