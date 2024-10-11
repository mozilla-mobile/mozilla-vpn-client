/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINSVCTHREAD_H
#define WINSVCTHREAD_H

#include <QString>
#include <QThread>

struct _SERVICE_STATUS;
struct SERVICE_STATUS_HANDLE__;

class WinSvcThread final : public QThread {
 public:
  WinSvcThread(const QString& name, QObject* parent = nullptr);
  ~WinSvcThread();

 private slots:
  void aboutToQuit();
  
 private:
  void run() override;
  void svcMain(const QStringList& arguments);
  void svcCtrlStop();

  static WinSvcThread* s_instance;
  static ulong svcCtrlHandler(ulong control, ulong type, void* event, void* ctx);

  struct SERVICE_STATUS_HANDLE__* m_svcCtrlHandle;
  struct _SERVICE_STATUS* m_serviceStatus;

  const QString m_serviceName;
};

#endif  // WINSVCTHREAD_H
