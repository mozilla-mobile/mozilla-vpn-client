/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "lottieprivatenavigator.h"

#include "lottieprivate.h"

LottiePrivateNavigator::LottiePrivateNavigator(LottiePrivate* parent)
    : QObject(parent), m_private(parent) {
  Q_ASSERT(parent);
}

QString LottiePrivateNavigator::userAgent() const {
  return m_private->userAgent();
}
