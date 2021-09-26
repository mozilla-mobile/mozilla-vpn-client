/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "modulemodel.h"
#include "leakdetector.h"
#include "logger.h"
#include "module.h"

namespace {
Logger logger(LOG_MODEL, "ModuleModel");
}

ModuleModel::ModuleModel() {
  MVPN_COUNT_CTOR(ModuleModel);

  m_modules = Module::createModules();
}

ModuleModel::~ModuleModel() { MVPN_COUNT_DTOR(ModuleModel); }

QHash<int, QByteArray> ModuleModel::roleNames() const {
  QHash<int, QByteArray> roles;
  roles[NameRole] = "name";
  roles[QmlViewRole] = "view";
  roles[QmlWidgetRole] = "widget";
  return roles;
}

int ModuleModel::rowCount(const QModelIndex&) const {
  return m_modules.count();
}

QVariant ModuleModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  switch (role) {
    case NameRole:
      return QVariant(m_modules.at(index.row())->name());

    case QmlViewRole:
      return QVariant(m_modules.at(index.row())->qmlView());

    case QmlWidgetRole:
      return QVariant(m_modules.at(index.row())->qmlWidget());

    default:
      return QVariant();
  }
}

QString ModuleModel::dashboardOrFirstView() const {
  if (m_modules.isEmpty()) {
    return QString();
  }

  return m_modules.count() > 1 ? "qrc:/ui/views/ViewDashboard.qml"
                               : m_modules.at(0)->qmlView();
}

void ModuleModel::loadModule(const QString& name) {
  logger.debug() << "Loading module" << name;

  for (Module* module : m_modules) {
    if (module->name() == name) {
      emit moduleNeeded(module->qmlView());
      return;
    }
  }

  logger.error() << "Unable to find module:" << name;
}
