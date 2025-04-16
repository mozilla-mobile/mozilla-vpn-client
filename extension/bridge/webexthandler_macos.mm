/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "webexthandler.h"

#include <QJsonObject>
#include <QString>

#include <libproc.h>
#include <unistd.h>

#import <AppKit/AppKit.h>

void WebExtHandler::proc_info(const QByteArray& msg) {
  Q_UNUSED(msg);

  // Gather the parent process details.
  pid_t parent = getppid();

  // TODO: Maybe the NSRunningApplication's bundle identifier or URL would be
  // more appropriate for the exe to report. We can fidle with this some more
  // when we get around to split tunneling support.
  char path[PROC_PIDPATHINFO_MAXSIZE];
  int ret = proc_pidpath(parent, path, PROC_PIDPATHINFO_MAXSIZE);

  // Send the response.
  QJsonObject obj;
  obj["pid"] = QJsonValue(parent);
  if (ret > 0) {
    obj["exe"] = QString(path);
  }
  writeJsonStdout(obj);
}

void WebExtHandler::start(const QByteArray& msg) {
  Q_UNUSED(msg);

  // Find the application to launch.
  NSWorkspace* ws = [NSWorkspace sharedWorkspace];
  NSString* bundleId = @"org.mozilla.macos.FirefoxVPN";
  NSURL* url = [ws URLForApplicationWithBundleIdentifier:bundleId];
  if (url == nil) {
    qWarning() << "Mozilla VPN application not found";
    return;
  }

  // Launch it.
  [ws openApplicationAtURL:url
             configuration:[NSWorkspaceOpenConfiguration configuration]
         completionHandler:^(NSRunningApplication* app, NSError* error) {
           if (error) {
             QString desc = QString::fromNSString(error.localizedDescription);
             qWarning() << "Mozilla VPN launch failed:" << desc;
           }
         }];
}
