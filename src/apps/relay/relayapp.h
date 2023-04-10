/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef RELAYAPP_H
#define RELAYAPP_H

#include "app.h"

class RelayApp final : public App {
 public:
  RelayApp();
  ~RelayApp();

  enum CustomScreen {
    ScreenHome,
    ScreenMessaging,
    ScreenCall,
    ScreenSettings,
  };
  Q_ENUM(CustomScreen);

  static RelayApp* instance();

  bool initialize();

 private:
  static void registerNavigationBarButtons();
};

#endif  // RELAYAPP_H
