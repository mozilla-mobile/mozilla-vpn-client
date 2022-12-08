/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDAPPLISTPROVIDER_H
#define ANDROIDAPPLISTPROVIDER_H

#include <applistprovider.h>

#include <QObject>

class AndroidAppListProvider : public AppListProvider {
  Q_OBJECT
 public:
  AndroidAppListProvider(QObject* parent);
  void getApplicationList() override;
};

#endif  // ANDROIDAPPLISTPROVIDER_H
