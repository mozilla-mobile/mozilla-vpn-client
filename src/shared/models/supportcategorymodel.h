/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SUPPORTCATEGORYMODEL_H
#define SUPPORTCATEGORYMODEL_H

#include <QAbstractListModel>
#include <QObject>

class SupportCategoryModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SupportCategoryModel)

 private:
  SupportCategoryModel() = default;

 public:
  enum SupportCategoryRoles {
    CategoryNameRole = Qt::UserRole + 1,
    UnlocalizedNameRole,
  };

  static SupportCategoryModel* instance();

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

};

#endif  // SUPPORTCATEGORYMODEL_H
