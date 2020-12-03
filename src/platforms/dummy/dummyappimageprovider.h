/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DUMMYAPPIMAGEPROVIDER_H
#define DUMMYAPPIMAGEPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>

class DummyAppImageProvider : public QQuickImageProvider, public QObject {
 public:
  DummyAppImageProvider(QObject* parent);
  ~DummyAppImageProvider();
  QPixmap requestPixmap(const QString& id, QSize* size,
                        const QSize& requestedSize) override;
};

#endif  // DUMMYAPPIMAGEPROVIDER_H
