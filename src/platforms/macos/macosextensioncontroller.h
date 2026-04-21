/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSEXTENSIONCONTROLLER_H
#define MACOSEXTENSIONCONTROLLER_H

#include "controllerimpl.h"

#include <QtDarwinHelpers>

Q_FORWARD_DECLARE_OBJC_CLASS(NETransparentProxyManager);
Q_FORWARD_DECLARE_OBJC_CLASS(NETunnelProviderSession);

class MacOSExtensionController final : public ControllerImpl {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MacOSExtensionController)

 public:
  MacOSExtensionController();
  ~MacOSExtensionController();

  void initialize(const Device* device, const Keys* keys) override;

  void activate(const InterfaceConfig& config, Controller::Reason reason) override;

  void deactivate() override;

  void checkStatus() override;

  bool splitTunnelSupported() const override { return true; }

 private slots:
  void extLoaderSuccess(int result);
  void extLoaderFailure(const QString& reason);
  void extNeedsApproval();

 private:
  static NSString* extIdentifier();

 private:
  void* m_delegate = nullptr;
  NETransparentProxyManager* m_manager = nullptr;
  NETunnelProviderSession* m_session = nullptr;
};

#endif  // MACOSEXTENSIONCONTROLLER_H
