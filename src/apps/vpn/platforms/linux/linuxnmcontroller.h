/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LINUXNMCONTROLLER_H
#define LINUXNMCONTROLLER_H

#include "controllerimpl.h"

#include <QObject>

struct _GAsyncResult;
struct _GObject;
struct _GCancellable;
struct _NMCLient;
struct _NMRemoteConnection;
struct _NMSettingWireGuard;

class LinuxNMController final : public ControllerImpl {
  Q_DISABLE_COPY_MOVE(LinuxNMController)

 public:
  LinuxNMController();
  ~LinuxNMController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const HopConnection& hop, const Device* device,
                const Keys* keys, Reason reason) override;

  void deactivate(Reason reason) override;

  void checkStatus() override;

  void getBackendLogs(std::function<void(const QString&)>&& callback) override;

  void cleanupBackendLogs() override;

 private:
  struct _GCancellable* m_cancellable;
  struct _NMClient* m_client = nullptr;
  struct _NMRemoteConnection* m_connection = nullptr;
  struct _NMSettingWireGuard* m_setting = nullptr;

 private:
  static void initializeCompleted(struct _GObject* obj,
                                  struct _GAsyncResult* res,
                                  void* user_data);
};

#endif  // LINUXNMCONTROLLER_H
