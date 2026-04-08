/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosapplistprovider.h"

#import <Foundation/Foundation.h>

#include <QDir>
#include <QDirIterator>
#include <QProcessEnvironment>
#include <QString>

#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger("MacOSAppListProvider");
}

MacOSAppListProvider::MacOSAppListProvider(QObject* parent)
    : AppListProvider(parent) {
  MZ_COUNT_CTOR(MacOSAppListProvider);
}

MacOSAppListProvider::~MacOSAppListProvider() {
  MZ_COUNT_DTOR(MacOSAppListProvider);
}

void MacOSAppListProvider::fetchEntries(const QString& appDir,
                                        QList<AppDescription>& out) {
  logger.debug() << "Fetch Application list from:" << appDir;

  QDirIterator iter(appDir, QDirIterator::FollowSymlinks);
  while (iter.hasNext()) {
    QString path = iter.next();

    logger.debug() << "Examining:" << path;
    NSBundle* bundle = [NSBundle bundleWithPath: path.toNSString()];
    if (bundle == nil) {
      continue;
    }
    logger.debug() << "Bundle identifier:" << bundle.bundleIdentifier;

    NSString* type = [bundle objectForInfoDictionaryKey:@"CFBundlePackageType"];
    if (type == nil) {
      continue;
    }
    logger.debug() << "Bundle type:" << type;
    if ([type compare:@"APPL"] != NSOrderedSame) {
      continue;
    }

    NSString* name = [bundle objectForInfoDictionaryKey:@"CFBundleName"];
    if (name == nil) {
      name = bundle.bundleIdentifier;
    }
    logger.debug() << "Bundle name:" << name;

    out.append({QString::fromNSString(bundle.bundleIdentifier),
                QString::fromNSString(name), false});
  }
}

void MacOSAppListProvider::getApplicationList() {
  QList<AppDescription> out;

  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  if (pe.contains("HOME")) {
    fetchEntries(pe.value("HOME") + "/Applications", out);
  }

  fetchEntries("/Applications", out);
  fetchEntries("/System/Applications", out);
  fetchEntries("/System/Cryptexes/App/System/Applications", out);

  emit newAppList(out);
}
