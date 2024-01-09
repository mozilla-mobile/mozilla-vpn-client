/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IOSWIDGETS_H
#define IOSWIDGETS_H
    
#include <QObject>


class IOSWidgets {
  Q_DISABLE_COPY_MOVE(IOSWidgets)

 public:
  IOSWidgets();
  ~IOSWidgets();

  void saveToUserDefaults();
};

#endif  // IOSLOGGER_H
