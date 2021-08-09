/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DUMMYIAPHANDLER_H
#define DUMMYIAPHANDLER_H

#include "iaphandler.h"

class DummyIAPHandler final : public IAPHandler {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(DummyIAPHandler)

 public:
  explicit DummyIAPHandler(QObject* parent);
  ~DummyIAPHandler();

 protected:
  void nativeRegisterProducts() override;
  void nativeStartSubscription(Product* product) override;

};

#endif  // DUMMYIAPHANDLER_H
