/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef FEEDBACKCATEGORYMODEL_H
#define FEEDBACKCATEGORYMODEL_H

#include <QAbstractListModel>

class SupportCategoryModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SupportCategoryModel)

 public:
  SupportCategoryModel();
  ~SupportCategoryModel();

  enum SupportCategoryRoles {
    CategoryNameRole = Qt::UserRole + 1,
    LocalizedNameRole,
  };

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;
};

#endif  // FEEDBACKCATEGORYMODEL_H
