/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testnavigator.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

#include "context/qmlengineholder.h"
#include "frontend/navigator.h"
#include "glean/generated/metrics.h"
#include "glean/mzglean.h"
#include "helper.h"
#include "localizer.h"
#include "mozillavpn.h"
#include "qmlpath.h"
#include "settings/settingsholder.h"

void TestNavigator::init() {
  m_settingsHolder = new SettingsHolder();

  // Glean needs to be initialized for every test because this test suite
  // includes telemetry tests.
  //
  // Glean operations are queued and applied once Glean is initialized.
  // If we only initialize it in the test that actually tests telemetry all
  // of the Glean operations from previous tests will be applied and mess with
  // the status of the test that actually is testing telemetry.
  //
  // Note: on tests Glean::initialize clears Glean's storage.
  MZGlean::initialize();
}

void TestNavigator::cleanup() { delete m_settingsHolder; }

void TestNavigator::testNavbarButtonTelemetry() {
  // Initialize
  Navigator* navigator = Navigator::instance();
  Localizer l;
  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  // Register screens
  Navigator::registerScreen(
      MozillaVPN::ScreenHome, Navigator::LoadPolicy::LoadPersistently,
      "qrc:/ui/screens/ScreenHome.qml", QVector<int>{},
      [](int*) -> int8_t { return 99; }, []() -> bool { return false; });

  Navigator::registerScreen(
      MozillaVPN::ScreenMessaging, Navigator::LoadPolicy::LoadPersistently,
      "qrc:/ui/screens/ScreenMessaging.qml", QVector<int>{},
      [](int*) -> int8_t { return 0; },
      []() -> bool {
        Navigator::instance()->requestScreen(MozillaVPN::ScreenHome,
                                             Navigator::ForceReload);
        return true;
      });

  Navigator::registerScreen(
      MozillaVPN::ScreenSettings, Navigator::LoadPolicy::LoadPersistently,
      "qrc:/ui/screens/ScreenSettings.qml", QVector<int>{},
      [](int*) -> int8_t { return 0; },
      []() -> bool {
        Navigator::instance()->requestScreen(MozillaVPN::ScreenHome,
                                             Navigator::ForceReload);
        return true;
      });

  int expectedHomeSelectedEventsCount = 0;
  int expectedMessagesSelectedEventsCount = 0;
  int expectedSettingsSelectedEventsCount = 0;

  // Setup UI - doesn't really matter what screen we start in
  navigator->requestScreen(MozillaVPN::ScreenHome);

  // Create QQuickItem that acts as a mock view and add it to the screens stack
  QQuickItem* view = new QQuickItem;
  navigator->addView(MozillaVPN::ScreenHome, QVariant::fromValue(view));

  // Test homeSelected event
  // Verify number of events is 0 before the test
  auto homeSelectedEvents =
      mozilla::glean::interaction::home_selected.testGetValue();

  QCOMPARE(homeSelectedEvents.length(), expectedHomeSelectedEventsCount);

  // Click the navbar home button
  Navigator::instance()->requestScreenFromBottomBar(MozillaVPN::ScreenHome);

  // Verify number of events is still 0 after the test
  // because the view does not have a telemetryScreenId property
  homeSelectedEvents =
      mozilla::glean::interaction::home_selected.testGetValue();

  QCOMPARE(homeSelectedEvents.length(), expectedHomeSelectedEventsCount);

  QString telemetryScreenId = "test-screen-id";

  // Give the mock view a telemetryScreenId which will be used as the events
  // extra key
  view->setProperty("telemetryScreenId", telemetryScreenId);

  // Test homeSelected event
  // Verify number of events is still 0 before the test
  homeSelectedEvents =
      mozilla::glean::interaction::home_selected.testGetValue();

  QCOMPARE(homeSelectedEvents.length(), expectedHomeSelectedEventsCount);

  // Click the navbar home button
  Navigator::instance()->requestScreenFromBottomBar(MozillaVPN::ScreenHome);
  expectedHomeSelectedEventsCount++;

  homeSelectedEvents =
      mozilla::glean::interaction::home_selected.testGetValue();

  // Verify number of events and event extras after test
  QCOMPARE(homeSelectedEvents.length(), expectedHomeSelectedEventsCount);

  auto homeSelectedEventsExtras = homeSelectedEvents[0]["extra"].toObject();

  QCOMPARE(homeSelectedEventsExtras["screen"].toString(), telemetryScreenId);

  QCOMPARE(mozilla::glean::interaction::home_selected.testGetNumRecordedErrors(
               ErrorType::InvalidValue),
           0);

  QCOMPARE(mozilla::glean::interaction::home_selected.testGetNumRecordedErrors(
               ErrorType::InvalidOverflow),
           0);

  // No need to go back to ScreenHome here because we did that as part of the
  // home button telemetry test above

  // Test messagesSelected event
  // Verify number of events is 0 before the test
  auto messagesSelectedEvents =
      mozilla::glean::interaction::messages_selected.testGetValue();

  QCOMPARE(messagesSelectedEvents.length(),
           expectedMessagesSelectedEventsCount);

  // Click the navbar messages button
  Navigator::instance()->requestScreenFromBottomBar(
      MozillaVPN::ScreenMessaging);
  expectedMessagesSelectedEventsCount++;

  messagesSelectedEvents =
      mozilla::glean::interaction::messages_selected.testGetValue();

  // Verify number of events and event extras after test
  QCOMPARE(messagesSelectedEvents.length(),
           expectedMessagesSelectedEventsCount);

  auto messagesSelectedEventsExtras =
      messagesSelectedEvents[0]["extra"].toObject();

  QCOMPARE(messagesSelectedEventsExtras["screen"].toString(),
           telemetryScreenId);

  QCOMPARE(
      mozilla::glean::interaction::messages_selected.testGetNumRecordedErrors(
          ErrorType::InvalidValue),
      0);

  QCOMPARE(
      mozilla::glean::interaction::messages_selected.testGetNumRecordedErrors(
          ErrorType::InvalidOverflow),
      0);

  // Go back to ScreenHome which has the mock view stacked on top of it before
  // starting the next test
  navigator->requestScreen(MozillaVPN::ScreenHome);

  // Test settingsSelected event
  // Verify number of events is 0 before the test
  auto settingsSelectedEvents =
      mozilla::glean::interaction::settings_selected.testGetValue();

  QCOMPARE(settingsSelectedEvents.length(),
           expectedSettingsSelectedEventsCount);

  // Click the navbar settings button
  Navigator::instance()->requestScreenFromBottomBar(MozillaVPN::ScreenSettings);
  expectedSettingsSelectedEventsCount++;

  settingsSelectedEvents =
      mozilla::glean::interaction::settings_selected.testGetValue();

  // Verify number of events and event extras after test
  QCOMPARE(settingsSelectedEvents.length(),
           expectedSettingsSelectedEventsCount);

  auto settingsSelectedEventsExtras =
      settingsSelectedEvents[0]["extra"].toObject();

  QCOMPARE(settingsSelectedEventsExtras["screen"].toString(),
           telemetryScreenId);

  QCOMPARE(
      mozilla::glean::interaction::settings_selected.testGetNumRecordedErrors(
          ErrorType::InvalidValue),
      0);

  QCOMPARE(
      mozilla::glean::interaction::settings_selected.testGetNumRecordedErrors(
          ErrorType::InvalidOverflow),
      0);

  view->deleteLater();
}

void TestNavigator::testNavbarButtonTelemetryNoLayers() {
  // Initialize
  Navigator* navigator = Navigator::instance();
  Localizer l;
  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  // Register screen
  Navigator::registerScreen(
      MozillaVPN::ScreenHome, Navigator::LoadPolicy::LoadPersistently,
      "qrc:/ui/screens/ScreenHome.qml", QVector<int>{},
      [](int*) -> int8_t { return 99; }, []() -> bool { return false; });

  // Click the home nav bar button to go to ScreenHome again
  navigator->requestScreen(MozillaVPN::ScreenHome);

  // Test homeSelected event and verify number of events is 0 because telemetry
  // is not recorded when there are no layers on the screen
  auto homeSelectedEvents =
      mozilla::glean::interaction::home_selected.testGetValue();
  QCOMPARE(homeSelectedEvents.length(), 0);
}

static TestNavigator s_testNavigator;
