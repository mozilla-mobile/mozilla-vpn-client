/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TEMPORARYDIR_H
#define TEMPORARYDIR_H

#include <QDir>
#include <QObject>
#include <QTemporaryDir>

class TemporaryDir final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(TemporaryDir)

 public:
  TemporaryDir(QObject* parent = nullptr);
  ~TemporaryDir();

  static void cleanupAll();

  QString errorString();
  QString filePath(const QString& fileName);
  bool isValid();
  QString path();

  // public to be used in unit-tests
  void fallback();

 private:
  enum {
    QT,
    Fallback,
  } m_state = QT;

  QTemporaryDir m_tmpDir;
  QDir m_fallbackDir;
};

#endif  // TEMPORARYDIR_H
