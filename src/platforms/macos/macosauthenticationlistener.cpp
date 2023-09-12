/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "macosauthenticationlistener.h"

#include <QCoreApplication>
#include <QFileOpenEvent>
#include <QUrlQuery>

#include "constants.h"
#include "leakdetector.h"
#include "logger.h"
#include "urlopener.h"

namespace {

Logger logger("MacosAuthenticationListener");

}  // anonymous namespace

MacosAuthenticationListener::MacosAuthenticationListener(QObject* parent)
    : DesktopAuthenticationListener(parent) {
  MZ_COUNT_CTOR(MacosAuthenticationListener);

  qApp->installEventFilter(this);
}

MacosAuthenticationListener::~MacosAuthenticationListener() {
  MZ_COUNT_DTOR(MacosAuthenticationListener);

  qApp->removeEventFilter(this);
}

bool MacosAuthenticationListener::eventFilter(QObject* obj, QEvent* event) {
  if (event->type() != QEvent::FileOpen) {
    return QObject::eventFilter(obj, event);
  }

  QFileOpenEvent* ev = static_cast<QFileOpenEvent*>(event);
  QUrl url = ev->url();
  if ((url.scheme() != Constants::DEEP_LINK_SCHEME) ||
      (url.authority() != "login")) {
    logger.warning() << "Received unknown URL:" << url.toString();
    return false;
  }
  if (url.path() != "/success") {
    logger.warning() << "Received unexpected auth endpoint:" << url.path();
    return false;
  }

  QUrlQuery query(url.query());
  if (!query.hasQueryItem("code")) {
    logger.warning() << "Received OAuth success, but no code was found.";
    return false;
  }

  emit completed(query.queryItemValue("code"));
  return false;
}
