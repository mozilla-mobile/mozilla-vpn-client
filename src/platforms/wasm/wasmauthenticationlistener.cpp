/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "wasmauthenticationlistener.h"

#include <QProcessEnvironment>
#include <QTimer>
#include <QUrlQuery>

#include "leakdetector.h"
#include "logger.h"
#include "urlopener.h"

namespace {

Logger logger("WasmAuthenticationListener");

}  // anonymous namespace

WasmAuthenticationListener::WasmAuthenticationListener(QObject* parent)
    : AuthenticationListener(parent) {
  MZ_COUNT_CTOR(WasmAuthenticationListener);
}

WasmAuthenticationListener::~WasmAuthenticationListener() {
  MZ_COUNT_DTOR(WasmAuthenticationListener);
}

void WasmAuthenticationListener::start(Task* task, const QString& codeChallenge,
                                       const QString& codeChallengeMethod,
                                       const QString& emailAddress) {
  Q_UNUSED(task);
  logger.debug() << "WasmAuthenticationListener initialize";

  QUrl url(createAuthenticationUrl(codeChallenge, codeChallengeMethod,
                                   emailAddress));

  QUrlQuery query(url.query());
  query.addQueryItem("utm_medium", "vpn-client");
  query.addQueryItem("utm_source", "wasm-signup-flow");

  url.setQuery(query);
  UrlOpener::instance()->setLastUrl(url.toString());

  // Unless we're in an automated test environment, mock out a successful auth.
  if (!isTesting()) {
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this,
            [this]() { emit completed("WASM"); });
    timer->start(2000);
  }
  // TODO: Food for future though - it's actually not improbable to do a real
  // auth via FxA here. We're already in a browser and we just need some way to
  // redirect back into the WASM client when done.
}

bool WasmAuthenticationListener::isTesting() const {
  QProcessEnvironment pe = QProcessEnvironment::systemEnvironment();
  return pe.contains("MVPN_WASM_TESTING") &&
         !pe.value("MVPN_WASM_TESTING").isEmpty();
}
