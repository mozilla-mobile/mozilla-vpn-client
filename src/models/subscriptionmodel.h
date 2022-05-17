/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SUBSCRIPTIONMODEL_H
#define SUBSCRIPTIONMODEL_H

#include <QAbstractListModel>

#include "subscriptionmodel.h"

class SubscriptionModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SubscriptionModel)

  Q_PROPERTY(bool initialized READ initialized NOTIFY changed)

 public:
  SubscriptionModel();
  ~SubscriptionModel();

  enum ModelRoles {
    KeyRole = Qt::UserRole + 1,
    ValuesRole,
  };

  // [[nodiscard]] bool fromJson(const QByteArray& json);
  Q_INVOKABLE bool fromJson(const QByteArray& json);

  bool initialized() const { return !m_rawJson.isEmpty(); }

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void changed();

 private:
  QByteArray m_rawJson;

  struct SubscriptionItem {
    QString m_key;
    QStringList m_values;
  };

  QList<SubscriptionItem> m_subscriptionList;
};

#endif  // SUBSCRIPTIONMODEL_H
