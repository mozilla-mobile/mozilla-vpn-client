/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NAVIGATORRELOADER_H
#define NAVIGATORRELOADER_H

#include <QObject>

class NavigatorReloader final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(NavigatorReloader)

 public:
  explicit NavigatorReloader(QObject* parent);
  ~NavigatorReloader();
};

#endif  // NAVIGATORRELOADER_H
