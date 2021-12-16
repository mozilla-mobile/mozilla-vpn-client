/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINSERVERDELEGATE_H
#define WINSERVERDELEGATE_H

#include <util/win/exception_handler_server.h>
#include <QObject>
#include "../../crashdata.h"

class WinServerDelegate : public QObject,
                          public crashpad::ExceptionHandlerServer::Delegate {
  Q_OBJECT
 public:
  WinServerDelegate();
  unsigned int ExceptionHandlerServerException(
      HANDLE process, crashpad::WinVMAddress exception_information_address,
      crashpad::WinVMAddress debug_critical_section_address) override;
  void ExceptionHandlerServerStarted() override;

 signals:
  void crashReported(std::shared_ptr<CrashData> data);
};

#endif  // WINSERVERDELEGATE_H
