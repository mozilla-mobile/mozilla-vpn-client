/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utils.h"

#include "app.h"
#include "appconstants.h"
#include "feature.h"
#include "logger.h"
#include "loghandler.h"
#include "tasks/createsupportticket/taskcreatesupportticket.h"
#include "urlopener.h"

#ifdef MZ_ANDROID
#  include "platforms/android/androidcommons.h"
#endif

#include <QApplication>
#include <QClipboard>

namespace {
Logger logger("Utils");
}

// static
Utils* Utils::instance() {
  static Utils s_instance;
  return &s_instance;
}

void MozillaVPN::createSupportTicket(const QString& email,
                                     const QString& subject,
                                     const QString& issueText,
                                     const QString& category) {
  logger.debug() << "Create support ticket";

  QString* buffer = new QString();
  QTextStream* out = new QTextStream(buffer);

  LogHandler::instance()->serializeLogs(
      out, [out, buffer, email, subject, issueText, category] {
        Q_ASSERT(out);
        Q_ASSERT(buffer);

        // buffer is getting copied by TaskCreateSupportTicket so we can delete
        // it afterwards
        Task* task = new TaskCreateSupportTicket(email, subject, issueText,
                                                 *buffer, category);
        delete buffer;
        delete out;

        // Support tickets can be created at anytime. Even during "critical"
        // operations such as authentication, account deletion, etc. Those
        // operations are often running in tasks, which would block the
        // scheduling of this new support ticket task execution if we used
        // `TaskScheduler::scheduleTask`. To avoid this, let's run this task
        // immediately and let's hope it does not fail.
        TaskScheduler::scheduleTaskNow(task);
      });
}

void Utils::exitForUnrecoverableError(const QString& reason) {
  Q_ASSERT(!reason.isEmpty());
  logger.error() << "Unrecoverable error detected: " << reason;
  App::instance()->quit();
}

// static
void Utils::storeInClipboard(const QString& text) {
  logger.debug() << "Store in clipboard";
  QApplication::clipboard()->setText(text);
}

// static
void Utils::crashTest() {
  logger.debug() << "Crashing Application";

#ifdef MZ_WINDOWS
  // Windows does not have "signals"
  //   qFatal("Ready to crash!") does not work as expected.
  // QT raises a debugmessage (in debugmode) - which we would handle
  // in release-mode however this end's with QT just doing a clean shutdown
  // so breakpad does not kick in.
  int i = 1;
  QString* ohno = (QString*)i--;
  ohno->at(1);
#else
  // On Linux/osx this generates a Sigabort, which is handled
  qFatal("Ready to crash!");
#endif
}

#ifdef MZ_ANDROID
// static
void Utils::launchPlayStore() {
  logger.debug() << "Launch Play Store";
  AndroidCommons::launchPlayStore();
}
#endif
