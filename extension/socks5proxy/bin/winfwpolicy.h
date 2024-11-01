/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINFWPOLICY_H
#define WINFWPOLICY_H

#include <QObject>

struct FWPM_SUBLAYER_CHANGE0_;

class WinFwPolicy final : public QObject {
  Q_OBJECT

 public:
  WinFwPolicy(QObject* parent = nullptr);
  ~WinFwPolicy();

  void restrictProxyPort(quint16 port);

 private:
  void setupKillswitch(const struct FWPM_SUBLAYER_CHANGE0_* change);

 private:
  void* m_fwEngineHandle = nullptr;
  void* m_fwChangeHandle = nullptr;
};

#endif  // WINFWPOLICY_H
