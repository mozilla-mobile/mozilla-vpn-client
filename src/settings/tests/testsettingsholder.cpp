/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testsettingsholder.h"

#define SETTING(type, toType, getter, setter, remover, has, key, defaultValue, \
                ...)                                                           \
  void TestSettingsHolder::testGetSetCheckRemove_##getter() {                  \
    SettingsHolder settingsHolder;                                             \
    SettingsManager::instance()->hardReset();                                  \
                                                                               \
    QSignalSpy spy(&settingsHolder, &SettingsHolder::getter##Changed);         \
                                                                               \
    QVariant v = QVariant(defaultValue);                                       \
    switch (v.metaType().id()) {                                               \
      case QMetaType::Bool: {                                                  \
        QVERIFY(!settingsHolder.has());                                        \
        QCOMPARE(QVariant(settingsHolder.getter()).toBool(), v.toBool());      \
                                                                               \
        bool expectedValue = !v.toBool();                                      \
        settingsHolder.setter(QVariant(expectedValue).toType());               \
        QCOMPARE(spy.count(), 1);                                              \
        QCOMPARE(QVariant(settingsHolder.getter()).toBool(), expectedValue);   \
        QVERIFY(settingsHolder.has());                                         \
                                                                               \
        settingsHolder.remover();                                              \
        QCOMPARE(spy.count(), 2);                                              \
        QVERIFY(!settingsHolder.has());                                        \
        return;                                                                \
      }                                                                        \
      case QMetaType::QByteArray: {                                            \
        QVERIFY(!settingsHolder.has());                                        \
        QCOMPARE(QVariant(settingsHolder.getter()).toByteArray(),              \
                 v.toByteArray());                                             \
                                                                               \
        QByteArray expectedValue = QString("abytearray").toUtf8();             \
        settingsHolder.setter(QVariant(expectedValue).toType());               \
        QCOMPARE(spy.count(), 1);                                              \
        QCOMPARE(QVariant(settingsHolder.getter()).toByteArray(),              \
                 expectedValue);                                               \
        QVERIFY(settingsHolder.has());                                         \
                                                                               \
        settingsHolder.remover();                                              \
        QCOMPARE(spy.count(), 2);                                              \
        QVERIFY(!settingsHolder.has());                                        \
        return;                                                                \
      }                                                                        \
      case QMetaType::QDateTime: {                                             \
        QVERIFY(!settingsHolder.has());                                        \
        QCOMPARE(QVariant(settingsHolder.getter()).toDateTime().toString(),    \
                 v.toDateTime().toString());                                   \
                                                                               \
        QDateTime expectedValue = QDateTime::fromMSecsSinceEpoch(0);           \
        settingsHolder.setter(QVariant(expectedValue).toType());               \
        QCOMPARE(spy.count(), 1);                                              \
        QCOMPARE(QVariant(settingsHolder.getter()).toDateTime().toString(),    \
                 expectedValue.toString());                                    \
        QVERIFY(settingsHolder.has());                                         \
                                                                               \
        settingsHolder.remover();                                              \
        QCOMPARE(spy.count(), 2);                                              \
        QVERIFY(!settingsHolder.has());                                        \
        return;                                                                \
      }                                                                        \
      case QMetaType::Int:                                                     \
      case QMetaType::LongLong: {                                              \
        QVERIFY(!settingsHolder.has());                                        \
        QCOMPARE(QVariant(settingsHolder.getter()).toInt(), v.toInt());        \
                                                                               \
        auto expectedValue = 42;                                               \
        settingsHolder.setter(QVariant(expectedValue).toType());               \
        QCOMPARE(spy.count(), 1);                                              \
        QCOMPARE(QVariant(settingsHolder.getter()).toInt(), expectedValue);    \
                                                                               \
        settingsHolder.remover();                                              \
        QCOMPARE(spy.count(), 2);                                              \
        QVERIFY(!settingsHolder.has());                                        \
        return;                                                                \
      }                                                                        \
      case QMetaType::QString: {                                               \
        QVERIFY(!settingsHolder.has());                                        \
        QCOMPARE(QVariant(settingsHolder.getter()).toString(), v.toString());  \
                                                                               \
        QString expectedValue = "astring";                                     \
        settingsHolder.setter(QVariant(expectedValue).toType());               \
        QCOMPARE(spy.count(), 1);                                              \
        QCOMPARE(QVariant(settingsHolder.getter()).toString(), expectedValue); \
                                                                               \
        settingsHolder.remover();                                              \
        QCOMPARE(spy.count(), 2);                                              \
        QVERIFY(!settingsHolder.has());                                        \
        return;                                                                \
      }                                                                        \
      case QMetaType::QStringList: {                                           \
        QVERIFY(!settingsHolder.has());                                        \
        QCOMPARE(QVariant(settingsHolder.getter()).toStringList().join(","),   \
                 v.toStringList().join(","));                                  \
                                                                               \
        QStringList expectedValue = QStringList({"a", "string", "list"});      \
        settingsHolder.setter(QVariant(expectedValue).toType());               \
        QCOMPARE(spy.count(), 1);                                              \
        QCOMPARE(QVariant(settingsHolder.getter()).toStringList().join(","),   \
                 expectedValue.join(","));                                     \
        QVERIFY(settingsHolder.has());                                         \
                                                                               \
        settingsHolder.remover();                                              \
        QCOMPARE(spy.count(), 2);                                              \
        QVERIFY(!settingsHolder.has());                                        \
        return;                                                                \
      }                                                                        \
      default: {                                                               \
        Q_ASSERT(false);                                                       \
      }                                                                        \
    }                                                                          \
  }

#include "settingslist.h"
#undef SETTING

#define EXPERIMENTAL_FEATURE(experimentId, experimentDescription, \
                             experimentSettings)                  \
  void TestSettingsHolder::testGetSet_##experimentId() {          \
    SettingsHolder settingsHolder;                                \
    auto group = settingsHolder.experimentId();                   \
                                                                  \
    QSignalSpy spy(group, &SettingGroup::changed);                \
                                                                  \
    if (experimentSettings.count() > 0) {                         \
      group->set(experimentSettings[0], QVariant("hey"));         \
      QCOMPARE(spy.count(), 1);                                   \
    }                                                             \
                                                                  \
    group->set("disalowedkey", QVariant("hey"));                  \
    QCOMPARE(spy.count(), 1);                                     \
  }

#include "feature/experimentalfeaturelist.h"
#undef EXPERIMENTAL_FEATURE

static TestSettingsHolder s_testSettingsHolder;
