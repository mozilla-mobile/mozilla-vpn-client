/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddonstatebase.h"

#include <QHash>
#include <QJsonObject>

#include "addons/state/addonstatebase.h"

/**
 * @brief Empty state class that doesn't do anything other than server as a
 * testing class for the Base abstract class.
 */
class StateSpy : public AddonStateBase {
 public:
  StateSpy(const QJsonObject& manifest) : AddonStateBase(manifest) {}
  StateSpy(const StateHash& manifest) : AddonStateBase(manifest) {}

  StateHash defaults() { return m_defaults; }

  QJsonValue getInternalReturnValue = QJsonValue();
  bool setInternalCalled = false;
  bool clearInternalCalled = false;

 private:
  QJsonValue getInternal(const QString& key) const override {
    return getInternalReturnValue;
  }

  void setInternal(const QString& key, const QJsonValue& value) override {
    setInternalCalled = true;
  }

  void clearInternal(const QString& key) override {
    clearInternalCalled = true;
  }
};

void TestAddonStateBase::testParseManifest_data() {
  QTest::addColumn<QJsonObject>("manifest");
  QTest::addColumn<StateHash>("hash");

  // Cases when the provided manifest is completely in the WRONG format.

  {
    QJsonObject obj;
    obj["prop1"] = 1;
    obj["prop2"] = "prop";
    obj["prop3"] = false;
    QTest::addRow("no_objects") << obj << StateHash();
  }

  {
    QJsonObject obj;
    obj["prop1"] = QJsonObject();
    QTest::addRow("missing_type_and_default") << obj << StateHash();
  }

  {
    QJsonObject prop;
    prop["default"] = false;
    QJsonObject obj;
    obj["prop"] = prop;
    QTest::addRow("missing_type") << obj << StateHash();
  }

  {
    QJsonObject prop1;
    prop1["type"] = "unsupported_type";
    prop1["default"] = false;
    QJsonObject prop2;
    prop2["type"] = 42;
    prop2["default"] = false;
    QJsonObject obj;
    obj["prop1"] = prop1;
    obj["prop2"] = prop2;
    QTest::addRow("invalid_type") << obj << StateHash();
  }

  {
    QJsonObject prop;
    prop["type"] = "boolean";
    QJsonObject obj;
    obj["prop"] = prop;
    QTest::addRow("missing_default") << obj << StateHash();
  }

  {
    QJsonObject boolProp;
    boolProp["type"] = "boolean";
    boolProp["default"] = 0;
    QJsonObject numProp;
    numProp["type"] = "number";
    numProp["default"] = "42";
    QJsonObject strProp;
    strProp["type"] = "string";
    strProp["default"] = QJsonObject();
    QJsonObject obj;
    obj["boolProp"] = boolProp;
    obj["numProp"] = numProp;
    obj["strProp"] = strProp;
    QTest::addRow("mismatched_type") << obj << StateHash();
  }

  // Cases when the provided manifest is completely in the RIGHT format.

  QTest::addRow("empty_manifest") << QJsonObject() << StateHash();

  {
    QJsonObject bool1;
    bool1["type"] = "boolean";
    bool1["default"] = false;
    QJsonObject bool2;
    bool2["type"] = "boolean";
    bool2["default"] = true;
    QJsonObject obj;
    obj["bool1"] = bool1;
    obj["bool2"] = bool2;
    QTest::addRow("bool_prop_correct")
        << obj << StateHash({{"bool1", false}, {"bool2", true}});
  }

  {
    QJsonObject str1;
    str1["type"] = "string";
    str1["default"] = "hello";
    QJsonObject str2;
    str2["type"] = "string";
    str2["default"] = "world";
    QJsonObject obj;
    obj["str1"] = str1;
    obj["str2"] = str2;
    QTest::addRow("str_prop_correct")
        << obj << StateHash({{"str1", "hello"}, {"str2", "world"}});
  }

  {
    QJsonObject num1;
    num1["type"] = "number";
    num1["default"] = 0;
    QJsonObject num2;
    num2["type"] = "number";
    num2["default"] = 2147483647;
    QJsonObject obj;
    obj["num1"] = num1;
    obj["num2"] = num2;
    QTest::addRow("num_prop_correct")
        << obj << StateHash({{"num1", 0}, {"num2", 2147483647}});
  }

  {
    QJsonObject boolProp;
    boolProp["type"] = "boolean";
    boolProp["default"] = false;
    QJsonObject numProp;
    numProp["type"] = "number";
    numProp["default"] = 24;
    QJsonObject strProp;
    strProp["type"] = "string";
    strProp["default"] = "hello/world";
    QJsonObject obj;
    obj["boolProp"] = boolProp;
    obj["numProp"] = numProp;
    obj["strProp"] = strProp;
    QTest::addRow("all_types_correct")
        << obj
        << StateHash({{"boolProp", false},
                      {"numProp", 24},
                      {"strProp", "hello/world"}});
  }

  // Cases when the provided manifest is completely PARTIALLY right.

  {
    QJsonObject oops;
    oops["type"] = "boolean";
    oops["default"] = "false";
    QJsonObject aBool;
    aBool["type"] = "boolean";
    aBool["default"] = true;
    QJsonObject obj;
    obj["oops"] = oops;
    obj["aBool"] = aBool;
    QTest::addRow("wrong_boolean_only") << obj << StateHash({{"aBool", true}});
  }

  {
    QJsonObject str1;
    str1["type"] = "string";
    str1["default"] = "hello";
    QJsonObject empty;
    QJsonObject obj;
    obj["str1"] = str1;
    obj["empty"] = empty;
    QTest::addRow("one_wrong_object") << obj << StateHash({{"str1", "hello"}});
  }

  {
    QJsonObject boolProp;
    boolProp["type"] = "boolean";
    boolProp["default"] = false;
    QJsonObject numProp;
    numProp["type"] = "number";
    numProp["default"] = 24;
    QJsonObject strProp;
    strProp["type"] = "string";
    strProp["default"] = "hello/world";
    QJsonObject obj;
    QJsonObject empty;
    obj["empty"] = empty;
    obj["boolProp"] = boolProp;
    obj["numProp"] = numProp;
    obj["strProp"] = strProp;
    QTest::addRow("all_types_correct_one_wrong")
        << obj
        << StateHash({{"boolProp", false},
                      {"numProp", 24},
                      {"strProp", "hello/world"}});
  }
}

void TestAddonStateBase::testParseManifest() {
  QFETCH(QJsonObject, manifest);
  QFETCH(StateHash, hash);

  StateSpy expectedState(hash);
  StateSpy actualState(manifest);
  QCOMPARE(expectedState.defaults(), actualState.defaults());
}

void TestAddonStateBase::testGet_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<StateHash>("defaults");
  QTest::addColumn<QJsonValue>("getInternalReturnValue");
  QTest::addColumn<QJsonValue>("expectedValue");

  StateHash defaults =
      StateHash({{"boolProp", false}, {"numProp", 0}, {"strProp", "aTest"}});

  // Cases when the default should be returned i.e. getInternalReturnValue ==
  // QJsonValue::Null.

  QTest::addRow("no_bool_recorded") << "boolProp" << defaults << QJsonValue()
                                    << QJsonValue(defaults.value("boolProp"));

  QTest::addRow("no_num_recorded") << "numProp" << defaults << QJsonValue()
                                   << QJsonValue(defaults.value("numProp"));

  QTest::addRow("no_str_recorded") << "strProp" << defaults << QJsonValue()
                                   << QJsonValue(defaults.value("strProp"));

  // Cases when the state should be returned i.e. getInternalReturnValue !==
  // QJsonValue::Null.

  QTest::addRow("bool_recorded")
      << "boolProp" << defaults << QJsonValue(true) << QJsonValue(true);

  QTest::addRow("num_recorded")
      << "numProp" << defaults << QJsonValue(42) << QJsonValue(42);

  QTest::addRow("str_recorded")
      << "strProp" << defaults << QJsonValue("ha!") << QJsonValue("ha!");

  // Error cases

  QTest::addRow("invalid_key")
      << "invalid" << defaults << QJsonValue("should never see this")
      << QJsonValue();
}

void TestAddonStateBase::testGet() {
  QFETCH(QString, key);
  QFETCH(StateHash, defaults);
  QFETCH(QJsonValue, getInternalReturnValue);
  QFETCH(QJsonValue, expectedValue);

  StateSpy state(defaults);
  state.getInternalReturnValue = getInternalReturnValue;

  QCOMPARE(state.get(key), expectedValue);
}

void TestAddonStateBase::testSet_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<QJsonValue>("value");
  QTest::addColumn<StateHash>("defaults");
  QTest::addColumn<bool>("setInternalCalled");

  StateHash defaults =
      StateHash({{"boolProp", false}, {"numProp", 0}, {"strProp", "aTest"}});

  // Cases when the set is called i.e. a valid key is being set with a valid
  // value is being set.

  QTest::addRow("set_bool_correctly")
      << "boolProp" << QJsonValue(true) << defaults << true;
  QTest::addRow("set_num_correctly")
      << "numProp" << QJsonValue(42) << defaults << true;
  QTest::addRow("set_str_correctly")
      << "strProp" << QJsonValue("ha!") << defaults << true;

  // Cases when the set is NOT called

  // 1. An invalid key is being set.

  QTest::addRow("invalid_key")
      << "invalid" << QJsonValue(true) << defaults << false;

  // 2. An invalid value is being set for a valid key.

  QTest::addRow("invalid_bool")
      << "boolProp" << QJsonValue() << defaults << false;

  QTest::addRow("invalid_num")
      << "numProp" << QJsonValue() << defaults << false;

  QTest::addRow("invalid_str")
      << "strProp" << QJsonValue() << defaults << false;
}

void TestAddonStateBase::testSet() {
  QFETCH(QString, key);
  QFETCH(QJsonValue, value);
  QFETCH(StateHash, defaults);
  QFETCH(bool, setInternalCalled);

  StateSpy state(defaults);
  state.set(key, value);

  QCOMPARE(state.setInternalCalled, setInternalCalled);
}

void TestAddonStateBase::testClear_data() {
  QTest::addColumn<QString>("key");
  QTest::addColumn<StateHash>("defaults");
  QTest::addColumn<bool>("clearInternalCalled");

  StateHash defaults =
      StateHash({{"boolProp", false}, {"numProp", 0}, {"strProp", "aTest"}});

  // Cases when the set is called i.e. a valid key is being set with a valid
  // value is being set.

  QTest::addRow("clear_bool_correctly") << "boolProp" << defaults << true;
  QTest::addRow("clear_num_correctly") << "numProp" << defaults << true;
  QTest::addRow("clear_str_correctly") << "strProp" << defaults << true;
  QTest::addRow("clear_all_correctly") << "" << defaults << true;

  // Cases when the set is NOT called i.e. an invalid key is being cleared.

  QTest::addRow("invalid_key") << "invalid" << defaults << false;
}

void TestAddonStateBase::testClear() {
  QFETCH(QString, key);
  QFETCH(StateHash, defaults);
  QFETCH(bool, clearInternalCalled);

  StateSpy state(defaults);
  state.clear(key);

  QCOMPARE(state.clearInternalCalled, clearInternalCalled);
}

static TestAddonStateBase s_testAddonStateBase;
