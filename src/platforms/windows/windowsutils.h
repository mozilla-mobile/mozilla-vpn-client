/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef WINDOWSUTILS_H
#define WINDOWSUTILS_H

#include <QString>

class QWindow;
class QImage;

class WindowsUtils final {
 public:
  static QString getErrorMessage();
  static QString getErrorMessage(quint32 code);
  static void windowsLog(const QString& msg);

  static bool getServiceStatus(const QString& name);

  // Returns the major version of Windows
  static QString windowsVersion();

  // Force an application crash for testing
  static void forceCrash();

  static void setTitleBarIcon(QWindow* window, const QImage& icon);
  static void setDockIcon(QWindow* window, const QImage& icon);
  /**
   *
   * @brief Set the Color for the titlebar, the color of the current theme
   * will be used. This only has impact on windows.
   *
   */
  static void updateTitleBarColor(QWindow* window, bool darkMode);
};

#endif  // WINDOWSUTILS_H
