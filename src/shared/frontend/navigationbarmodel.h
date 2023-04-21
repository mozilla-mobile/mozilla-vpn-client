/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NAVIGATIONBARMODEL_H
#define NAVIGATIONBARMODEL_H

#include <QAbstractListModel>

#include "navigationbarbutton.h"

class NavigationBarModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NavigationBarModel)

  Q_PROPERTY(int count READ count NOTIFY modelChanged);

 public:
  static NavigationBarModel* instance();

  ~NavigationBarModel();

  enum ModelRoles {
    ButtonRole = Qt::UserRole + 1,
  };

  void appendButton(NavigationBarButton* button);

  int count() const { return static_cast<int>(m_buttons.length()); }

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void modelChanged();

 private:
  explicit NavigationBarModel(QObject* parent);

  void resetButtonSelection();

 private:
  QList<NavigationBarButton*> m_buttons;
};

#endif  // NAVIGATIONBARMODEL_H
