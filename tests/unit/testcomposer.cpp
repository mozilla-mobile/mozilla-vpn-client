/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testcomposer.h"
#include "../../src/addons/addon.h"
#include "../../src/addons/addonmessage.h"
#include "../../src/composer/composer.h"
#include "../../src/composer/composerblockbutton.h"
#include "../../src/composer/composerblocktext.h"
#include "../../src/composer/composerblocktitle.h"
#include "../../src/composer/composerblockunorderedlist.h"
#include "../../src/composer/composerblockorderedlist.h"
#include "../../src/settingsholder.h"
#include "../../src/qmlengineholder.h"
#include "helper.h"

#include <QQmlApplicationEngine>

void TestComposer::generic_data() {
  QTest::addColumn<QJsonObject>("json");
  QTest::addColumn<bool>("result");
  QTest::addColumn<int>("items");

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = 42;

  QJsonObject obj;
  obj["message"] = content;
  QTest::addRow("invalid type") << obj << false;

  content["blocks"] = QJsonArray();
  obj["message"] = content;
  QTest::addRow("empty blocks") << obj << true << 0;

  content["blocks"] = QJsonArray{42};
  obj["message"] = content;
  QTest::addRow("invalid block type") << obj << false;

  content["blocks"] = QJsonArray{QJsonObject()};
  obj["message"] = content;
  QTest::addRow("unknown block type") << obj << false;
}

void TestComposer::generic() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QFETCH(QJsonObject, json);

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", json);

  QFETCH(bool, result);
  QCOMPARE(!!message, result);

  if (!result) {
    return;
  }

  Composer* composer = qobject_cast<AddonMessage*>(message)->composer();
  QVERIFY(!!composer);

  const QList<ComposerBlock*>& blocks = composer->blocks();

  QFETCH(int, items);
  QCOMPARE(blocks.length(), items);
}

void TestComposer::button_data() {
  QTest::addColumn<QJsonObject>("button");
  QTest::addColumn<bool>("result");
  QTest::addColumn<ComposerBlockButton::Style>("style");

  QJsonObject button;
  button["content"] = "foo bar";
  QTest::addRow("object no type") << button << false;

  button["type"] = "button";
  QTest::addRow("button without id") << button << false;

  button["id"] = "b";
  QTest::addRow("button without javascript") << button << false;

  button["javascript"] = "404.js";
  QTest::addRow("not-existing js file") << button << false;

  button["javascript"] = ":/addons_test/button1.js";
  QTest::addRow("not a function") << button << false;

  button["javascript"] = ":/addons_test/button2.js";
  QTest::addRow("exception") << button << false;

  button["javascript"] = ":/addons_test/button3.js";
  QTest::addRow("good") << button << true
                        << ComposerBlockButton::Style::Primary;

  button["javascript"] = ":/addons_test/button4.js";
  QTest::addRow("good with exception")
      << button << true << ComposerBlockButton::Style::Primary;

  button["style"] = "invalid";
  QTest::addRow("invalid button stlye") << button << false;

  button["style"] = "primary";
  QTest::addRow("primary button")
      << button << true << ComposerBlockButton::Style::Primary;

  button["style"] = "destructive";
  QTest::addRow("destructive button")
      << button << true << ComposerBlockButton::Style::Destructive;

  button["style"] = "link";
  QTest::addRow("link button")
      << button << true << ComposerBlockButton::Style::Link;
}

void TestComposer::button() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QFETCH(QJsonObject, button);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray{button};

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);

  QFETCH(bool, result);
  QCOMPARE(!!message, result);

  if (!result) {
    return;
  }

  Composer* composer = qobject_cast<AddonMessage*>(message)->composer();
  QVERIFY(!!composer);

  const QList<ComposerBlock*>& blocks = composer->blocks();
  QCOMPARE(blocks.length(), 1);

  const ComposerBlockButton* block =
      qobject_cast<const ComposerBlockButton*>(blocks.at(0));
  QVERIFY(!!block);

  QFETCH(ComposerBlockButton::Style, style);
  QCOMPARE(block->style(), style);

  QCOMPARE(block->property("text").toString(), "foo bar");

  block->click();
}

void TestComposer::text_data() {
  QTest::addColumn<QJsonObject>("text");
  QTest::addColumn<bool>("result");

  QJsonObject text;
  text["content"] = "foo bar";
  QTest::addRow("object no type") << text << false;

  text["type"] = "text";
  QTest::addRow("text without id") << text << false;

  text["id"] = "b";
  QTest::addRow("good") << text << true;
}

void TestComposer::text() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QFETCH(QJsonObject, text);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray{text};

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);

  QFETCH(bool, result);
  QCOMPARE(!!message, result);

  if (!result) {
    return;
  }

  Composer* composer = qobject_cast<AddonMessage*>(message)->composer();
  QVERIFY(!!composer);

  const QList<ComposerBlock*>& blocks = composer->blocks();
  QCOMPARE(blocks.length(), 1);

  const ComposerBlockText* block =
      qobject_cast<const ComposerBlockText*>(blocks.at(0));
  QVERIFY(!!block);

  QCOMPARE(block->property("text").toString(), "foo bar");
}

void TestComposer::title_data() {
  QTest::addColumn<QJsonObject>("title");
  QTest::addColumn<bool>("result");

  QJsonObject title;
  title["content"] = "foo bar";
  QTest::addRow("object no type") << title << false;

  title["type"] = "title";
  QTest::addRow("title without id") << title << false;

  title["id"] = "b";
  QTest::addRow("good") << title << true;
}

void TestComposer::title() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QFETCH(QJsonObject, title);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray{title};

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);

  QFETCH(bool, result);
  QCOMPARE(!!message, result);

  if (!result) {
    return;
  }

  Composer* composer = qobject_cast<AddonMessage*>(message)->composer();
  QVERIFY(!!composer);

  const QList<ComposerBlock*>& blocks = composer->blocks();
  QCOMPARE(blocks.length(), 1);

  const ComposerBlockTitle* block =
      qobject_cast<const ComposerBlockTitle*>(blocks.at(0));
  QVERIFY(!!block);

  QCOMPARE(block->property("title").toString(), "foo bar");
}

void TestComposer::unorderedList_data() {
  QTest::addColumn<QJsonObject>("unorderedList");
  QTest::addColumn<bool>("result");
  QTest::addColumn<int>("items");

  QJsonObject unorderedList;
  QTest::addRow("object no type") << unorderedList << false;

  unorderedList["type"] = "ulist";
  QTest::addRow("unorderedList without id") << unorderedList << false;

  unorderedList["id"] = "b";
  QTest::addRow("unordered list without content") << unorderedList << false;

  unorderedList["content"] = "foo bar";
  QTest::addRow("invalid content type") << unorderedList << false;

  unorderedList["content"] = QJsonArray();
  QTest::addRow("empty content") << unorderedList << true << 0;

  unorderedList["content"] = QJsonArray{42};
  QTest::addRow("invalid sub-content type") << unorderedList << false;

  QJsonObject obj;
  unorderedList["content"] = QJsonArray{obj};
  QTest::addRow("no sub-content id") << unorderedList << false;

  obj["content"] = "WOW";
  obj["id"] = "id";
  unorderedList["content"] = QJsonArray{obj};
  QTest::addRow("one block") << unorderedList << true << 1;

  unorderedList["content"] = QJsonArray{obj, obj};
  QTest::addRow("two blocks") << unorderedList << true << 2;
}

void TestComposer::unorderedList() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QFETCH(QJsonObject, unorderedList);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray{unorderedList};

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);

  QFETCH(bool, result);
  QCOMPARE(!!message, result);

  if (!result) {
    return;
  }

  Composer* composer = qobject_cast<AddonMessage*>(message)->composer();
  QVERIFY(!!composer);

  const QList<ComposerBlock*>& blocks = composer->blocks();
  QCOMPARE(blocks.length(), 1);

  const ComposerBlockUnorderedList* block =
      qobject_cast<const ComposerBlockUnorderedList*>(blocks.at(0));
  QVERIFY(!!block);

  QStringList subBlocks = block->property("subBlocks").toStringList();

  QFETCH(int, items);
  QCOMPARE(subBlocks.length(), items);

  for (int i = 0; i < items; ++i) {
    QCOMPARE(subBlocks[i], "WOW");
  }
}

void TestComposer::orderedList_data() {
  QTest::addColumn<QJsonObject>("orderedList");
  QTest::addColumn<bool>("result");
  QTest::addColumn<int>("items");

  QJsonObject orderedList;
  QTest::addRow("object no type") << orderedList << false;

  orderedList["type"] = "olist";
  QTest::addRow("orderedList without id") << orderedList << false;

  orderedList["id"] = "b";
  QTest::addRow("ordered list without content") << orderedList << false;

  orderedList["content"] = "foo bar";
  QTest::addRow("invalid content type") << orderedList << false;

  orderedList["content"] = QJsonArray();
  QTest::addRow("empty content") << orderedList << true << 0;

  orderedList["content"] = QJsonArray{42};
  QTest::addRow("invalid sub-content type") << orderedList << false;

  QJsonObject obj;
  orderedList["content"] = QJsonArray{obj};
  QTest::addRow("no sub-content id") << orderedList << false;

  obj["content"] = "WOW";
  obj["id"] = "id";
  orderedList["content"] = QJsonArray{obj};
  QTest::addRow("one block") << orderedList << true << 1;

  orderedList["content"] = QJsonArray{obj, obj};
  QTest::addRow("two blocks") << orderedList << true << 2;
}

void TestComposer::orderedList() {
  MozillaVPN vpn;
  SettingsHolder settingsHolder;

  QQmlApplicationEngine engine;
  QmlEngineHolder qml(&engine);

  QFETCH(QJsonObject, orderedList);

  QJsonObject content;
  content["id"] = "foo";
  content["blocks"] = QJsonArray{orderedList};

  QJsonObject obj;
  obj["message"] = content;

  QObject parent;
  Addon* message = AddonMessage::create(&parent, "foo", "bar", "name", obj);

  QFETCH(bool, result);
  QCOMPARE(!!message, result);

  if (!result) {
    return;
  }

  Composer* composer = qobject_cast<AddonMessage*>(message)->composer();
  QVERIFY(!!composer);

  const QList<ComposerBlock*>& blocks = composer->blocks();
  QCOMPARE(blocks.length(), 1);

  const ComposerBlockUnorderedList* block =
      qobject_cast<const ComposerBlockUnorderedList*>(blocks.at(0));
  QVERIFY(!!block);

  QStringList subBlocks = block->property("subBlocks").toStringList();

  QFETCH(int, items);
  QCOMPARE(subBlocks.length(), items);

  for (int i = 0; i < items; ++i) {
    QCOMPARE(subBlocks[i], "WOW");
  }
}

static TestComposer s_testComposer;
