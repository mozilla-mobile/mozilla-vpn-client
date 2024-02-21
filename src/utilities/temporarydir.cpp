/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "temporarydir.h"

#include <QStandardPaths>
#include <QUuid>

#include "leakdetector.h"
#include "logging/logger.h"

namespace {
Logger logger("TemporaryDir");

constexpr const char* TMP_FOLDER = "tmp";

QString rootAppFolder() {
#ifdef MZ_WASM
  // https://wiki.qt.io/Qt_for_WebAssembly#Files_and_local_file_system_access
  return "/";
#elif defined(UNIT_TEST)
  return QStandardPaths::writableLocation(QStandardPaths::TempLocation);
#else
  return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#endif
}
}  // namespace

TemporaryDir::TemporaryDir(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(TemporaryDir);
}

TemporaryDir::~TemporaryDir() {
  MZ_COUNT_DTOR(TemporaryDir);

  if (m_state == Fallback && !m_fallbackDir.removeRecursively()) {
    logger.debug() << "Failed to remove the fallback dir"
                   << m_fallbackDir.path();
  }
}

QString TemporaryDir::errorString() {
  if (isValid()) {
    return QString();
  }

  if (m_state == QT) {
    return m_tmpDir.errorString();
  }

  return "Unable to create the fallback directory";
}

QString TemporaryDir::filePath(const QString& fileName) {
  if (!isValid()) {
    return QString();
  }

  if (m_state == QT) {
    return m_tmpDir.filePath(fileName);
  }

  return m_fallbackDir.filePath(fileName);
}

void TemporaryDir::fallback() {
  m_fallbackDir = QDir(rootAppFolder());

  if (!m_fallbackDir.exists(TMP_FOLDER) && !m_fallbackDir.mkpath(TMP_FOLDER)) {
    logger.error() << "Unable to create the tmp fallback folder"
                   << rootAppFolder();
    return;
  }

  if (!m_fallbackDir.cd(TMP_FOLDER)) {
    logger.error() << "Unable to open the tmp fallback folder"
                   << m_fallbackDir.path();
    return;
  }

  int retry = 0;
  while (true) {
    if (++retry > 10) {
      logger.error()
          << "Unable to find a valid UUID for the tmp fallback folder"
          << m_fallbackDir.path();
      return;
    }

    QByteArray uuid = QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
    if (m_fallbackDir.exists(uuid)) {
      continue;
    }

    if (!m_fallbackDir.mkpath(uuid)) {
      logger.error() << "Unable to create the tmp fallback folder"
                     << m_fallbackDir.path() << uuid;
      return;
    }

    if (!m_fallbackDir.cd(uuid)) {
      logger.error() << "Unable to open the tmp fallback folder"
                     << m_fallbackDir.path() << uuid;
      return;
    }

    break;
  }

  m_state = Fallback;
}

bool TemporaryDir::isValid() {
  if (m_state == QT) {
    if (m_tmpDir.isValid()) {
      return true;
    }

    fallback();
  }

  if (m_state == QT) {
    return m_tmpDir.isValid();
  }

  return m_fallbackDir.exists();
}

QString TemporaryDir::path() {
  if (!isValid()) {
    return QString();
  }

  if (m_state == QT) {
    return m_tmpDir.path();
  }

  return m_fallbackDir.path();
}

// static
void TemporaryDir::cleanupAll() {
  QDir fallbackDir(rootAppFolder());

  if (!fallbackDir.exists(TMP_FOLDER)) {
    return;
  }

  if (!fallbackDir.cd(TMP_FOLDER)) {
    logger.error() << "Unable to open the tmp fallback folder"
                   << fallbackDir.path();
    return;
  }

  if (!fallbackDir.removeRecursively()) {
    logger.debug() << "Failed to remove the fallback dir" << fallbackDir.path();
  }
}
