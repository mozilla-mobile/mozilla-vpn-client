/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WHATSNEWMODEL_H
#define WHATSNEWMODEL_H

#include <QAbstractListModel>

class Feature;

class WhatsNewModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(WhatsNewModel)

  Q_PROPERTY(bool hasUnseenFeature READ hasUnseenFeature NOTIFY
                 hasUnseenFeatureChanged)

 public:
  WhatsNewModel();
  ~WhatsNewModel();

  enum ModelRoles {
    RoleFeature,
  };

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  QStringList featureIds() const;

  bool hasUnseenFeature();

  Q_INVOKABLE void markFeaturesAsSeen();

 signals:
  void hasUnseenFeatureChanged();

 private:
  void initialize();

  QList<Feature*> m_featurelist;
};

#endif  // WHATSNEWMODEL_H
