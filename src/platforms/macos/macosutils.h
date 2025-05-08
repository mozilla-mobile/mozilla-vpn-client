/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MACOSUTILS_H
#define MACOSUTILS_H

#include <QObject>
#include <QString>

class MacOSUtils final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(MacOSUtils)

 public:
  MacOSUtils(QObject* parent = nullptr) : QObject(parent){};

  static MacOSUtils* instance();

  Q_INVOKABLE void openSystemSettingsLoginItems();

  static NSString* appId();

  static QString computerName();

  static void enableLoginItem(bool startAtBoot);

  static void setDockClickHandler();
  static void setStatusBarTextColor();

  static void hideDockIcon();
  static void showDockIcon();

  static void patchNSStatusBarSetImageForBigSur();
};

#endif  // MACOSUTILS_H
