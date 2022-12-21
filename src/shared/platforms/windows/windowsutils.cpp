/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowsutils.h"

#include <QSettings>
#include <QSysInfo>

constexpr const int WINDOWS_11_BUILD =
    22000;  // Build Number of the first release win 11 iso

// Static
QString WindowsUtils::windowsVersion() {
  /* The Tradegy of Getting a somewhat working windows version:
    - GetVersion() -> deprecated and Reports win 8.1 for MozillaVPN... its tied
    to some .exe flags
    - NetWkstaGetInfo -> Reports Windows 10 on windows 11
    There is also the regirstry HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows
    NT\CurrentVersion
    -> CurrentMajorVersion reports 10 on win 11
    -> CurrentBuild seems to be correct, so lets infer it
  */

  QSettings regCurrentVersion(
      "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
      QSettings::NativeFormat);

  int buildNr = regCurrentVersion.value("CurrentBuild").toInt();
  if (buildNr >= WINDOWS_11_BUILD) {
    return "11";
  }
  return QSysInfo::productVersion();
}
