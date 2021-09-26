/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "module.h"
#include "models/feature.h"
#include "logger.h"

#include <QCoreApplication>

QVector<std::function<Module*()>> Module::s_moduleCreators;

namespace {
Logger logger(LOG_MODEL, "Module");
}  // namespace

Module::Module(const QString& name, const QString& featureId,
               const QString& qmlView, const QString& qmlWidget)
    : QObject(qApp),
      m_name(name),
      m_featureId(featureId),
      m_qmlView(qmlView),
      m_qmlWidget(qmlWidget) {
  logger.debug() << "Initializing module" << name;
}

// static
QList<Module*> Module::createModules() {
  QList<Module*> list;
  for (auto i = s_moduleCreators.begin(); i != s_moduleCreators.end(); ++i) {
    Module* m = (*i)();
    Q_ASSERT(m);

    const Feature* feature = Feature::get(m->featureId());
    Q_ASSERT(feature);

    if (feature->isSupported()) {
      list.append(m);
    }
  }

  return list;
}
