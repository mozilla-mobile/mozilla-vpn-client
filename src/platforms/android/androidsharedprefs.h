/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDSHAREDPREFS_H
#define ANDROIDSHAREDPREFS_H

#include "authenticationlistener.h"

#include <QObject>

class AndroidSharedPrefs final {
  Q_DISABLE_COPY_MOVE(AndroidSharedPrefs)

 public:
  AndroidSharedPrefs();
  ~AndroidSharedPrefs();

  /*
   * Returns all the Shared Prefrences Files that are available
   */
  static QList<QString> GetPrefFiles();
  static QVariant GetValue(const QString& filenName, const QString& prefKey);
};

#endif  // ANDROIDSHAREDPREFS_H
