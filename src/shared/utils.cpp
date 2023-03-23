/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utils.h"

#include "app.h"
#include "appconstants.h"
#include "feature.h"
#include "logger.h"
#include "loghandler.h"
#include "urlopener.h"

#ifdef MZ_ANDROID
#  include "platforms/android/androidiaphandler.h"
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

void Utils::openAppStoreReviewLink() {
  Q_ASSERT(Feature::get(Feature::Feature_appReview)->isSupported());

#if defined(MZ_IOS)
  UrlOpener::instance()->openUrl(AppConstants::APPLE_STORE_REVIEW_URL);
#elif defined(MZ_ANDROID)
  UrlOpener::instance()->openUrl(AppConstants::GOOGLE_PLAYSTORE_URL);
#endif
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
  PurchaseHandler* purchaseHandler = PurchaseHandler::instance();
  static_cast<AndroidIAPHandler*>(purchaseHandler)->launchPlayStore();
}
#endif
