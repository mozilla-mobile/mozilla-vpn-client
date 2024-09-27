/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddon.h"

#include <QQmlApplicationEngine>
#include <QTemporaryFile>

#include "addons/addon.h"
#include "addons/addonmessage.h"
#include "addons/addonproperty.h"
#include "addons/addonpropertylist.h"
#include "addons/conditionwatchers/addonconditionwatcherfeaturesenabled.h"
#include "addons/conditionwatchers/addonconditionwatchergroup.h"
#include "addons/conditionwatchers/addonconditionwatcherjavascript.h"
#include "addons/conditionwatchers/addonconditionwatcherlocales.h"
#include "addons/conditionwatchers/addonconditionwatchertimeend.h"
#include "addons/conditionwatchers/addonconditionwatchertimestart.h"
#include "addons/conditionwatchers/addonconditionwatchertriggertimesecs.h"
#include "addons/manager/addonmanager.h"
#include "feature/feature.h"
#include "feature/featuremodel.h"
#include "glean/generated/metrics.h"
#include "glean/mzglean.h"
#include "helper.h"
#include "localizer.h"
#include "qmlengineholder.h"
#include "qtglean.h"
#include "settingsholder.h"
#include "systemtraynotificationhandler.h"

void TestAddon::init() {
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
  MZGlean::initialize("testing");
}

void TestAddon::cleanup() { delete m_settingsHolder; }

void TestAddon::message_notification_data() {
  SettingsHolder settingsHolder;
  Localizer l;

  QObject parent;
  SystemTrayNotificationHandler nh(&parent);

  QTest::addColumn<QString>("title");
  QTest::addColumn<QString>("message");
  QTest::addColumn<QString>("actual_title");
  QTest::addColumn<QString>("actual_message");

  TestHelper::resetLastSystemNotification();
  // Message is created for the first time,
  // but user is not logged in, no  message sent
  QTest::addRow("not-logged-in")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  // Message is created for the first time,
  // user is not logged in and message is disabled, no  message sent
  AddonMessage* disabledPreLoginMessage = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message1.json"));
  emit AddonManager::instance()->addonCreated(disabledPreLoginMessage);
  QTest::addRow("not-logged-in-disabled")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  // Mock a user login.
  TestHelper::resetLastSystemNotification();
  settingsHolder.setToken("Hello World");
  App::instance()->setState(App::StateMain);
  // A login should not trigger any messages either.
  QTest::addRow("login") << QString() << QString()
                         << TestHelper::lastSystemNotification.title
                         << TestHelper::lastSystemNotification.message;

  // Message received pre login is enabled post login, message sent
  TestHelper::resetLastSystemNotification();
  // Message is later enabled
  disabledPreLoginMessage->enable();
  QTest::addRow("enable-post-login")
      << QString("Test Message 1 - Title")
      << QString("Test Message 1 - Subtitle")
      << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is created for the first time, notification should be sent
  AddonMessage* message = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message2.json"));
  emit AddonManager::instance()->addonCreated(message);
  QTest::addRow("do-show") << QString("Test Message 2 - Title")
                           << QString("Test Message 2 - Subtitle")
                           << TestHelper::lastSystemNotification.title
                           << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is created for the second time, notification should not be sent
  Addon* anotherAddon = Addon::create(&parent, ":/addons_test/message2.json");
  emit AddonManager::instance()->addonCreated(anotherAddon);
  QTest::addRow("do-not-show")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is marked as read and we re-attempt to send a notification
  message->markAsRead();
  NotificationHandler::instance()->maybeAddonNotification(message);
  QTest::addRow("message-is-read")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Another message is created for the first time
  AddonMessage* anotherMessage = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message3.json"));
  emit AddonManager::instance()->addonCreated(anotherMessage);
  QTest::addRow("do-show-2") << QString("Test Message 3 - Title")
                             << QString("Test Message 3 - Subtitle")
                             << TestHelper::lastSystemNotification.title
                             << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is dismissed and we re-attempt to send a notification
  anotherMessage->dismiss();
  NotificationHandler::instance()->maybeAddonNotification(anotherMessage);
  QTest::addRow("message-dismissed")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is created but due to its conditions it's not enabled
  AddonMessage* disabledMessage = static_cast<AddonMessage*>(
      Addon::create(&parent, ":/addons_test/message4.json"));
  emit AddonManager::instance()->addonCreated(disabledMessage);
  QTest::addRow("message-loaded-disabled")
      << QString() << QString() << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  TestHelper::resetLastSystemNotification();
  // Message is later enabled
  disabledMessage->enable();
  QTest::addRow("message-enabled")
      << QString("Test Message 4 - Title")
      << QString("Test Message 4 - Subtitle")
      << TestHelper::lastSystemNotification.title
      << TestHelper::lastSystemNotification.message;

  App::instance()->setState(App::StateInitialize);
}

void TestAddon::message_notification() {
  QFETCH(QString, title);
  QFETCH(QString, message);
  QFETCH(QString, actual_title);
  QFETCH(QString, actual_message);

  QCOMPARE(actual_title, title);
  QCOMPARE(actual_message, message);
}

static TestAddon s_testAddon;
