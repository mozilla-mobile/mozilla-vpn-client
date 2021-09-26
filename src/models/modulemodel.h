/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULEMODEL_H
#define MODULEMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "module.h"

class ModuleModel final : public QAbstractListModel {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ModuleModel)

  Q_PROPERTY(QString dashboardOrFirstView READ dashboardOrFirstView CONSTANT)

 public:
  ModuleModel();
  ~ModuleModel();

  enum ModelRoles {
    NameRole = Qt::UserRole + 1,
    QmlViewRole,
    QmlWidgetRole,
  };

  QString dashboardOrFirstView() const;

  Q_INVOKABLE void loadModule(const QString& name);

  // QAbstractListModel methods

  QHash<int, QByteArray> roleNames() const override;

  int rowCount(const QModelIndex&) const override;

  QVariant data(const QModelIndex& index, int role) const override;

 signals:
  void moduleNeeded(const QString& qmlView);

 private:
  QList<Module*> m_modules;
};

#endif  // MODULEMODEL_H
