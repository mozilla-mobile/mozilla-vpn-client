/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "utils.h"

#include "appconstants.h"
#include "feature.h"
#include "logger.h"
#include "loghandler.h"
#include "urlopener.h"

#ifdef MZ_ANDROID
#  include "platforms/android/androidiaphandler.h"
#endif
#ifdef MZ_WINDOWS
#  include "platforms/windows/windowsutils.h"
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
  WindowsUtils::forceCrash();
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
