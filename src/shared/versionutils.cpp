/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "versionutils.h"

#include <QRegularExpression>

// static
int VersionUtils::compareVersions(const QString& a, const QString& b) {
  if (a == b) {
    return 0;
  }

  if (a.isEmpty()) {
    return 1;
  }

  if (b.isEmpty()) {
    return -1;
  }

  static QRegularExpression re("[^0-9a-z.]");

  QStringList aParts;
  qsizetype aMatchLength = a.indexOf(re);
  aParts = (aMatchLength < 0) ? a.split(".") : a.left(aMatchLength).split(".");

  QStringList bParts;
  qsizetype bMatchLength = b.indexOf(re);
  bParts = (bMatchLength < 0) ? b.split(".") : b.left(bMatchLength).split(".");

  // Normalize by appending zeros as necessary.
  while (aParts.length() < 3) aParts.append("0");
  while (bParts.length() < 3) bParts.append("0");

  // Major version number.
  for (uint32_t i = 0; i < 3; ++i) {
    int aDigit = aParts[i].toInt();
    int bDigit = bParts[i].toInt();
    if (aDigit != bDigit) {
      return aDigit < bDigit ? -1 : 1;
    }
  }

  return 0;
}

// static
QString VersionUtils::stripMinor(const QString& a) {
  QStringList aParts;

  if (!a.isEmpty()) {
    static QRegularExpression re("[^0-9a-z.]");
    qsizetype matchLength = a.indexOf(re);
    aParts = (matchLength < 0) ? a.split(".") : a.left(matchLength).split(".");
  }

  while (aParts.length() < 3) aParts.append("0");

  while (aParts.length() > 2) aParts.removeLast();

  aParts.append("0");
  return aParts.join(".");
}
