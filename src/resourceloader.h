/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <QObject>
#include <QTimer>

class QDir;

class ResourceLoader final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(ResourceLoader)

 public:
  static ResourceLoader* instance();

  ~ResourceLoader();

  void checkForUpdates(const QByteArray& data);

 private:
  ResourceLoader(QObject* parent);

  // For loading:
  static void loadResources();
  static bool loadQRB(const QDir& dir);
  static bool recoverMigration(QDir& dir);

  // For updates:
  static bool cleanupCacheFolder(QDir& cacheDir);
  static bool deserializeData(const QByteArray& data, const QDir& cacheDir);
  static bool bringToProd(QDir& cacheDir);

  // Common utils:
  static bool validateManifestAndFiles(const QDir& dir);

 private:
  QTimer m_timer;

#ifdef UNIT_TEST
  friend class TestResourceLoader;
#endif
};

#endif  // RESOURCELOADER_H
