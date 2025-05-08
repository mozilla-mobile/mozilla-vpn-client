/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "inspectorhotreloader.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include <QWindow>

#include "frontend/navigator.h"
#include "logger.h"
#include "networkrequest.h"
#include "qmlengineholder.h"
#include "tasks/function/taskfunction.h"

namespace {
Logger logger("QMLHotReload");
}

InspectorHotreloader::InspectorHotreloader(QQmlEngine* target)
    : m_target(target) {
  m_target->addUrlInterceptor(this);
}

QUrl InspectorHotreloader::intercept(
    const QUrl& url, QQmlAbstractUrlInterceptor::DataType type) {
  logger.debug() << "Requested: " << url.fileName();
  if (m_announced_files.contains(url.fileName())) {
    logger.debug() << "Redirect! : "
                   << m_announced_files[url.fileName()].toString();
    return m_announced_files[url.fileName()];
  }
  return url;
}

void InspectorHotreloader::annonceReplacedFile(const QUrl& path) {
  logger.debug() << "Announced redirect! : " << path.fileName() << " as ->"
                 << path.toString();

  if (path.scheme() != "file" && path.scheme() != "qrc") {
    logger.error() << "Unexpected File Scheme in: " << path.toString();
    return;
  }
  // If it's a file, just load it!
  m_target->clearComponentCache();
  m_announced_files.insert(path.fileName(), path);
  Navigator::instance()->reloadCurrentScreen();
  return;
}

void InspectorHotreloader::resetAllFiles() {
  logger.debug() << "Resetting hot reloaded files";

  m_target->clearComponentCache();
  m_announced_files.clear();
  Navigator::instance()->reloadCurrentScreen();
}

void InspectorHotreloader::reloadWindow() {
  auto engineHolder = QmlEngineHolder::instance();
  QQmlApplicationEngine* engine =
      static_cast<QQmlApplicationEngine*>(engineHolder->engine());
  // Here is the main QML file.
  if (!engineHolder->hasWindow()) {
    logger.error() << "No Window to reload";
    return;
  }
  logger.error() << "Closing and full reloading window";

  int x = 0, y = 0;
  // We may have multiple closed windows still in here.
  // So best to go over them all.
  QList<QObject*> rootObjects = engine->rootObjects();
  for (auto* rootObject : rootObjects) {
    QWindow* maybeWindow = qobject_cast<QWindow*>(rootObject);
    if (maybeWindow) {
      if (maybeWindow->isVisible()) {
        x = maybeWindow->x();
        y = maybeWindow->y();
      }
      maybeWindow->close();
    }
  }
  const QUrl url(QStringLiteral("qrc:/qt/qml/Mozilla/VPN/main.qml"));
  engine->load(url);
  auto newWindow = qobject_cast<QWindow*>(engine->rootObjects().last());
  if (newWindow) {
    if (x != 0 && y != 0) {
      newWindow->setX(x);
      newWindow->setY(y);
    }
  }
}
