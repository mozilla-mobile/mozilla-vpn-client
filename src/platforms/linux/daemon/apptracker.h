/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef APPTRACKER_H
#define APPTRACKER_H

#include <QDBusObjectPath>
#include <QFileSystemWatcher>
#include <QString>

class QDBusInterface;

class AppTracker final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(AppTracker)

 public:
  explicit AppTracker(uint userid, const QDBusObjectPath& path,
                      QObject* parent = nullptr);
  ~AppTracker();

 signals:
  void appLaunched(const QString& name, int rootpid);

 private slots:
  void gtkLaunchEvent(const QByteArray& appid, const QString& display,
                      qlonglong pid, const QStringList& uris,
                      const QVariantMap& extra);

  void cgroupsChanged(const QString& directory);

 private:
  const uint m_userid;
  const QDBusObjectPath m_objectPath;
  QDBusInterface* m_interface;

  QString m_cgroupPath;
  QFileSystemWatcher m_cgroupWatcher;

  QStringList m_cgroupScopes;
};

#endif  // APPTRACKER_H
