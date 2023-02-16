/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testaddonsessionstate.h"

#include "addons/state/addonsessionstate.h"

void TestAddonSessionState::testGetAndSet() {
  QJsonObject numProp;
  numProp["type"] = "number";
  numProp["default"] = 10;
  QJsonObject manifest;
  manifest["numProp"] = numProp;

  AddonSessionState* state = AddonSessionState::fromManifest(manifest);

  QCOMPARE(state->get("numProp"), 10);

  state->set("numProp", QJsonValue(11));
  QCOMPARE(state->get("numProp"), 11);
}

void TestAddonSessionState::testClear() {
  QJsonObject numProp;
  numProp["type"] = "number";
  numProp["default"] = 10;
  QJsonObject manifest;
  manifest["numProp"] = numProp;

  AddonSessionState* state = AddonSessionState::fromManifest(manifest);

  QCOMPARE(state->get("numProp"), 10);

  // Clear without having recorded anything.
  state->clear("numProp");
  QCOMPARE(state->get("numProp"), 10);

  state->set("numProp", QJsonValue(11));
  QCOMPARE(state->get("numProp"), 11);

  // Clear after having recorded.
  state->clear("numProp");
  QCOMPARE(state->get("numProp"), 10);
}

void TestAddonSessionState::testClearAll() {
  QJsonObject prop1;
  prop1["type"] = "number";
  prop1["default"] = 10;
  QJsonObject prop2;
  prop2["type"] = "number";
  prop2["default"] = 11;
  QJsonObject prop3;
  prop3["type"] = "number";
  prop3["default"] = 12;
  QJsonObject manifest;
  manifest["prop1"] = prop1;
  manifest["prop2"] = prop2;
  manifest["prop3"] = prop3;

  AddonSessionState* state = AddonSessionState::fromManifest(manifest);

  QCOMPARE(state->get("prop1"), 10);
  QCOMPARE(state->get("prop2"), 11);
  QCOMPARE(state->get("prop3"), 12);
  // Clear all without having recorded anything.
  state->clear();
  QCOMPARE(state->get("prop1"), 10);
  QCOMPARE(state->get("prop2"), 11);
  QCOMPARE(state->get("prop3"), 12);

  state->set("prop1", QJsonValue(1));
  state->set("prop2", QJsonValue(2));
  QCOMPARE(state->get("prop1"), 1);
  QCOMPARE(state->get("prop2"), 2);
  QCOMPARE(state->get("prop3"), 12);
  // Clear all having recorded part of the properties anything.
  state->clear();
  QCOMPARE(state->get("prop1"), 10);
  QCOMPARE(state->get("prop2"), 11);
  QCOMPARE(state->get("prop3"), 12);

  state->set("prop1", QJsonValue(0));
  state->set("prop2", QJsonValue(0));
  state->set("prop3", QJsonValue(0));
  QCOMPARE(state->get("prop1"), 0);
  QCOMPARE(state->get("prop2"), 0);
  QCOMPARE(state->get("prop3"), 0);
  // Clear after having recorded all props
  state->clear();
  QCOMPARE(state->get("prop1"), 10);
  QCOMPARE(state->get("prop2"), 11);
  QCOMPARE(state->get("prop3"), 12);
}

static TestAddonSessionState s_testAddonSessionState;
