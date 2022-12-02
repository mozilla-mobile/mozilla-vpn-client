/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MODULEHOLDER_H
#define MODULEHOLDER_H

#include <QMap>
#include <QQmlPropertyMap>

#define MODULE(key, obj) static constexpr const char* Module_##key = #key;
#include "modulelist.h"
#undef MODULE

class Module;

class ModuleHolder final : public QQmlPropertyMap {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ModuleHolder)

 public:
  static ModuleHolder* instance();

  ~ModuleHolder();

  void registerModule(const QString& moduleName, Module* moduleObj);

 private:
  explicit ModuleHolder(QObject* parent);

 private:
  QMap<QString, Module*> m_modules;
};

#endif  // MODULEHOLDER_H
