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

#include "context/qmlengineholder.h"
#include "frontend/navigator.h"
#include "frontend/navigatorreloader.h"
#include "logging/logger.h"
#include "networkrequest.h"
#include "tasks/function/taskfunction.h"

namespace {
Logger logger("QMLHotReload");
}

InspectorHotreloader::InspectorHotreloader(QQmlEngine* target)
    : m_target(target) {
  m_target->addUrlInterceptor(this);
  new NavigatorReloader(qApp);
  QDir dataDir(
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
  m_qml_folder = dataDir.absoluteFilePath("hot_reload");

  QDir dir(m_qml_folder);
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  // On shutdown cleanup all things.
  QObject::connect(qApp, &QCoreApplication::aboutToQuit, [this] {
    QDir dir(m_qml_folder);
    if (dir.exists()) {
      dir.removeRecursively();
    }
  });
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

  if (path.scheme() != "http" && path.scheme() != "file" &&
      path.scheme() != "qrc") {
    logger.error() << "Unexpected File Scheme in: " << path.toString();
    return;
  }
  if (path.scheme() == "file" || path.scheme() == "qrc") {
    // If it's a file, just load it!
    m_target->clearComponentCache();
    m_announced_files.insert(path.fileName(), path);
    Navigator::instance()->reloadCurrentScreen();
    return;
  }
  // If it is a remote file, download to a temp file and then come back to
  // announce it!
  fetchAndAnnounce(path);
}

void InspectorHotreloader::fetchAndAnnounce(const QUrl& path) {
  if (path.scheme() != "http") {
    logger.error() << "Unexpected File Scheme in: " << path.toString();
    return;
  }
  TaskFunction* dummy_task = new TaskFunction([]() {});
  NetworkRequest* request = new NetworkRequest(dummy_task, 200);
  request->get(path.toString());

  QObject::connect(
      request, &NetworkRequest::requestFailed,
      [dummy_task](QNetworkReply::NetworkError error, const QByteArray&) {
        dummy_task->deleteLater();
        logger.error() << "Get qml content failed" << error;
      });

  QObject::connect(
      request, &NetworkRequest::requestCompleted,
      [this, path, dummy_task](const QByteArray& data) {
        dummy_task->deleteLater();
        auto temp_path = QString("%1/%2").arg(m_qml_folder, path.fileName());
        auto temp_file = new QFile(temp_path);
        temp_file->open(QIODevice::WriteOnly);
        if (!temp_file->write(data)) {
          logger.warning() << "Unable to write to file:"
                           << temp_file->fileName();
          return;
        }
        if (!temp_file->flush()) {
          logger.warning() << "Unable to flush to file:"
                           << temp_file->fileName();
          return;
        }
        temp_file->close();
        QFileInfo info(temp_path);
        annonceReplacedFile(QUrl::fromLocalFile(info.absoluteFilePath()));
      });
}

void InspectorHotreloader::resetAllFiles() {
  logger.debug() << "Resetting hot reloaded files";

  QDir dir(m_qml_folder);
  if (dir.exists()) {
    logger.debug() << "Removing hot reloaded files from disk";
    dir.removeRecursively();
  }
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
  const QUrl url(QStringLiteral("qrc:/ui/main.qml"));
  engine->load(url);
  auto newWindow = qobject_cast<QWindow*>(engine->rootObjects().last());
  if (newWindow) {
    if (x != 0 && y != 0) {
      newWindow->setX(x);
      newWindow->setY(y);
    }
  }
}
