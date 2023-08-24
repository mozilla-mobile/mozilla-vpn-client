/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSAUTHENTICATIONLISTENER_H
#define MACOSAUTHENTICATIONLISTENER_H

#include <QEvent>

#include "tasks/authenticate/desktopauthenticationlistener.h"

class MacosAuthenticationListener final : public DesktopAuthenticationListener {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MacosAuthenticationListener)

 public:
  explicit MacosAuthenticationListener(QObject* parent);
  ~MacosAuthenticationListener();

 private:
  bool eventFilter(QObject* obj, QEvent* event) override;
};

#endif  // MACOSAUTHENTICATIONLISTENER_H
