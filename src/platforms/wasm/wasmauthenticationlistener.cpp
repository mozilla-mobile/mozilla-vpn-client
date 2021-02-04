/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmauthenticationlistener.h"
#include "leakdetector.h"
#include "networkrequest.h"
#include "logger.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDesktopServices>

namespace {

Logger logger(LOG_MAIN, "WasmAuthenticationListener");

}  // anonymous namespace

WasmAuthenticationListener::WasmAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MVPN_COUNT_CTOR(WasmAuthenticationListener);
}

WasmAuthenticationListener::~WasmAuthenticationListener() {
  MVPN_COUNT_DTOR(WasmAuthenticationListener);
}

void WasmAuthenticationListener::start(MozillaVPN* vpn, QUrl& url,
                                       QUrlQuery& query) {
  logger.log() << "WasmAuthenticationListener initialize";
  Q_UNUSED(vpn);
  Q_UNUSED(url);
  Q_UNUSED(query);

  QTimer* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, [this]() { emit completed("WASM"); });
  timer->start(2000);
}
