/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSSTATUSICON_H
#define MACOSSTATUSICON_H

#include <QMenu>
#include <QObject>

class MacOSStatusIcon final : public QObject {
 public:
  MacOSStatusIcon(QObject* parent);
  ~MacOSStatusIcon();

 public:
  void setStatusBarIcon(QString iconUrl);
  void setStatusBarIndicatorColor(QColor indicatorColor);
  void setStatusBarMenu(NSMenu* statusBarMenu);
};

#endif  // MACOSSTATUSICON_H
