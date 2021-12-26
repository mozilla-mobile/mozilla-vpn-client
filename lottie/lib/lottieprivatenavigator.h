/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef LOTTIEPRIVATENAVIGATOR_H
#define LOTTIEPRIVATENAVIGATOR_H

#include <QObject>

class LottiePrivate;

// A simple "DOM navigator" implementation.

class LottiePrivateNavigator final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString userAgent READ userAgent CONSTANT)

 public:
  explicit LottiePrivateNavigator(LottiePrivate* parent);

  QString userAgent() const;

 private:
  LottiePrivate* m_private = nullptr;
};

#endif  // LOTTIEPRIVATENAVIGATOR_H
