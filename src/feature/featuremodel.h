/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEATUREMODEL_H
#define FEATUREMODEL_H

#include <QAbstractListModel>
#include <QObject>
#include <QPair>

class Feature;

class FeatureModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(FeatureModel)

 private:
  FeatureModel() = default;

 public:
  enum ModelRoles {
    FeatureRole = Qt::UserRole + 1,
  };

  static FeatureModel* instance();

  void updateFeatureList(const QByteArray& data);

  // QAbstractListModel methods
  QHash<int, QByteArray> roleNames() const override;
  int rowCount(const QModelIndex&) const override;
  QVariant data(const QModelIndex& index, int role) const override;

  Q_INVOKABLE void toggle(const QString& feature);
  Q_INVOKABLE QObject* get(const QString& feature);

 private:
  static QPair<QStringList, QStringList> parseFeatures(
      const QJsonValue& features);
  static QPair<QStringList, QStringList> parseExperimentalFeatures(
      const QJsonValue& experimentalFeatures);
};

#endif  // FEATUREMODEL_H
