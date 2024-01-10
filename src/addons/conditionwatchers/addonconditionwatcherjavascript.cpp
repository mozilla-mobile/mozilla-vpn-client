/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "addonconditionwatcherjavascript.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QQmlEngine>

#include "addons/addon.h"
#include "addons/addonapi.h"
#include "context/qmlengineholder.h"
#include "logging/logger.h"
#include "utilities/leakdetector.h"

namespace {
Logger logger("AddonConditionWatcherJavascript");
}

// static
AddonConditionWatcher* AddonConditionWatcherJavascript::maybeCreate(
    Addon* addon, const QString& javascript) {
  if (javascript.isEmpty()) {
    return nullptr;
  }

  QFileInfo manifestFileInfo(addon->manifestFileName());
  QDir addonPath = manifestFileInfo.dir();

  QFile file(addonPath.filePath(javascript));
  if (!file.open(QIODevice::ReadOnly)) {
    logger.debug() << "Unable to open the javascript file" << javascript;
    return nullptr;
  }

  QJSValue output =
      QmlEngineHolder::instance()->engine()->evaluate(file.readAll());
  if (output.isError()) {
    logger.debug() << "Execution throws an error:" << output.toString();
    return nullptr;
  }

  if (!output.isCallable()) {
    logger.debug() << "The condition should be a callable function";
    return nullptr;
  }

  return new AddonConditionWatcherJavascript(addon, output);
}

AddonConditionWatcherJavascript::AddonConditionWatcherJavascript(
    Addon* addon, const QJSValue& function)
    : AddonConditionWatcher(addon) {
  MZ_COUNT_CTOR(AddonConditionWatcherJavascript);

  QJSEngine* engine = QmlEngineHolder::instance()->engine();
  QJSValue api = engine->newQObject(addon->api());
  QJSValue self = engine->newQObject(this);

  QJSValue output = function.call(QJSValueList{api, self});
  if (output.isError()) {
    logger.debug() << "Execution of the watcher function failed"
                   << output.toString();
  }
}

AddonConditionWatcherJavascript::~AddonConditionWatcherJavascript() {
  MZ_COUNT_DTOR(AddonConditionWatcherJavascript);
}

bool AddonConditionWatcherJavascript::conditionApplied() const {
  return m_currentStatus;
}

void AddonConditionWatcherJavascript::enable() {
  if (m_currentStatus) {
    return;
  }

  m_currentStatus = true;
  emit conditionChanged(m_currentStatus);
}

void AddonConditionWatcherJavascript::disable() {
  if (!m_currentStatus) {
    return;
  }

  m_currentStatus = false;
  emit conditionChanged(m_currentStatus);
}
