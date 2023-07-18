/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsentryadapter.h"

#include "feature.h"
#include "helper.h"
#include "models/featuremodel.h"
#include "networkrequest.h"
#include "sentry/sentryadapter.h"
#include "settingsholder.h"
#include "simplenetworkmanager.h"

/* Test's that Sentry will, (if
 * no configuration is present)
 *
 * Create a task that fetches a
 * Crashreporting info form
 *  /api/crashreporting.
 */
void TestSentryAdapter::init_creates_task() {
  SettingsHolder settingsHolder;
  SimpleNetworkManager snm;
  bool networkRequest_fired = false;
  if (!Feature::get(Feature::Feature_sentry)->isSupported()) {
    FeatureModel::instance()->toggle("sentry");
  }
  NetworkRequest::setRequestHandler(
      // DELETE
      [](NetworkRequest*) { return false; },
      // GET
      [&networkRequest_fired](NetworkRequest* req) {
        if (req->url().path().endsWith("/crashreporting")) {
          networkRequest_fired = true;
        }
        return true;
      },
      // POST
      [](NetworkRequest*, const QByteArray&) { return false; },
      // POST with an io device
      [](NetworkRequest*, QIODevice*) { return false; });
  SentryAdapter::instance()->init();
  QEventLoop loop;
  loop.processEvents(QEventLoop::AllEvents, 2000);
  QVERIFY(networkRequest_fired);
};

/**
 * @brief When the configuration is Present,
 * Sentry should *NOT* have created a request to guardian.
 */
void TestSentryAdapter::init_no_task_when_config_in_settings() {
  SettingsHolder settingsHolder;
  settingsHolder.clear();
  settingsHolder.setSentryDSN("abcd");
  settingsHolder.setSentryEndpoint("https://not.sentry.url");
  SimpleNetworkManager snm;
  bool networkRequest_fired = false;
  if (!Feature::get(Feature::Feature_sentry)->isSupported()) {
    FeatureModel::instance()->toggle("sentry");
  }
  NetworkRequest::setRequestHandler(
      [](NetworkRequest*) { return false; },
      [&networkRequest_fired](NetworkRequest* req) {
        if (req->url().path().endsWith("/crashreporting")) {
          networkRequest_fired = true;
        }
        return true;
      },
      [](NetworkRequest*, const QByteArray&) { return false; },
      [](NetworkRequest*, QIODevice*) { return false; });
  SentryAdapter::instance()->init();
  QEventLoop loop;
  loop.processEvents(QEventLoop::AllEvents, 2000);

  QVERIFY(!networkRequest_fired);
  QVERIFY(SentryAdapter::instance()->ready());
}

static TestSentryAdapter s_TestSentryAdapter;