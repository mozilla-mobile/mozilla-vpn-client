/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidsharedprefs.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "QDir"
#include "QVariant"
#include <QtXml>

namespace {
Logger logger(LOG_ANDROID, "AndroidSharedPrefs");

#ifdef QT_DEBUG
const QString SHARED_PREF_FOLDER =
    "/data/data/org.mozilla.firefox.vpn.debug/shared_prefs";
#else
const QString SHARED_PREF_FOLDER =
    "/data/data/org.mozilla.firefox.vpn/shared_prefs";
#endif
}  // namespace

AndroidSharedPrefs::AndroidSharedPrefs() {
  MVPN_COUNT_CTOR(AndroidSharedPrefs);
}

AndroidSharedPrefs::~AndroidSharedPrefs() {
  MVPN_COUNT_DTOR(AndroidSharedPrefs);
}

QList<QString> AndroidSharedPrefs::GetPrefFiles() {
  QDir prefFolder(SHARED_PREF_FOLDER);
  return prefFolder.entryList();
}
/**
 * @brief AndroidSharedPrefs::GetValue
 * @param fileName - The Shared Prefrence file to open e.g "example.xml"
 * @param prefKey  - The prefrence key to get
 * @return Returns the value of the Pref, QVariant::Invalid on failure
 */
QVariant AndroidSharedPrefs::GetValue(const QString& fileName,
                                      const QString& prefKey) {
  QDir prefFolder(SHARED_PREF_FOLDER);
  if (!prefFolder.exists()) {
    logger.error() << "shared_prefs folder not found: " << prefFolder.path();
    return QVariant::Invalid;
  }

  QFile prefFile(prefFolder.absoluteFilePath(fileName));
  if (!prefFile.exists()) {
    logger.error() << "pref file not found: " << prefFile.fileName();
    return QVariant::Invalid;
  }
  if (!prefFile.open(QIODevice::ReadOnly)) {
    logger.error() << "failed to open pref file: " << prefFile.fileName();
    return QVariant::Invalid;
  }

  /* Android shared prefrences format:
   * <map>
   *   <string name="key"> value </string>
   *   ..
   * </map>
   */

  QDomDocument xmlBOM;
  xmlBOM.setContent(&prefFile);
  QDomElement mapNode = xmlBOM.documentElement();
  QDomNodeList stringElementList = mapNode.childNodes();

  for (int x = 0; x < stringElementList.count(); x++) {
    QDomNode stringNode =
        stringElementList.at(x);  // <string name="key">value</string>
    auto attribName = stringNode.attributes().namedItem("name");
    if (attribName.nodeValue() == prefKey) {
      QString val = stringNode.nodeValue();
      auto textNode = stringNode.firstChild();
      if (!textNode.isText()) {
        logger.warning() << "Non TextNode Value? " << textNode.nodeValue();
      }
      QString value = textNode.nodeValue();
      prefFile.close();
      return QVariant(value);
    }
  }

  prefFile.close();
  logger.error() << "Key was not found in the Prefrences file " << prefKey;
  return QVariant::Invalid;
}
