/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testdocument.h"
#include "../../lib/lottieprivate.h"
#include "../../lib/lottieprivatedocument.h"

#include <QJSEngine>

void TestDocument::readyState() {
  LottiePrivate p;
  LottiePrivateDocument doc(&p);

  QCOMPARE(doc.readyState(), "complete");
}

void TestDocument::createElement() {
  QJSEngine engine;
  LottiePrivate::initialize(&engine, "Foo 1.0");

  LottiePrivate p;
  LottiePrivateDocument doc(&p);

  {
    QJSValue error = doc.createElement("foo");
    QVERIFY(error.isError());
  }

  {
    QJSValue value = doc.createElement("canvas");
    QVERIFY(value.isNull());
  }

  QQuickItem canvasObj, containerObj;
  p.setCanvasAndContainer(&canvasObj, &containerObj);

  // Creating a canvas, we return the existing one.
  {
    QJSValue value = doc.createElement("canvas");
    QVERIFY(value.isObject());
    QCOMPARE(value.toQObject(), &canvasObj);
  }

  // Creating a canvas, we return the existing one.
  {
    QJSValue value = doc.createElement("CaNvAs");
    QVERIFY(value.isObject());
    QCOMPARE(value.toQObject(), &canvasObj);
  }
}

void TestDocument::getElementByTagName() {
  QJSEngine engine;
  LottiePrivate::initialize(&engine, "Foo 1.0");

  LottiePrivate p;
  LottiePrivateDocument doc(&p);

  {
    QJSValue array = doc.getElementsByTagName("foo");
    QVERIFY(array.isArray());
    QCOMPARE(array.property("length").toInt(), 0);
  }

  {
    QJSValue array = doc.getElementsByTagName("canvas");
    QVERIFY(array.isArray());
    QCOMPARE(array.property("length").toInt(), 0);
  }

  QQuickItem canvasObj, containerObj;
  p.setCanvasAndContainer(&canvasObj, &containerObj);

  {
    QJSValue array = doc.getElementsByTagName("canvas");
    QVERIFY(array.isArray());
    QCOMPARE(array.property("length").toInt(), 1);

    QJSValue value = array.property("0");
    QVERIFY(value.isObject());
    QCOMPARE(value.toQObject(), &canvasObj);
  }

  {
    QJSValue array = doc.getElementsByTagName("CaNvAs");
    QVERIFY(array.isArray());
    QCOMPARE(array.property("length").toInt(), 1);

    QJSValue value = array.property("0");
    QVERIFY(value.isObject());
    QCOMPARE(value.toQObject(), &canvasObj);
  }
}

void TestDocument::getElementByClassName() {
  QJSEngine engine;
  LottiePrivate::initialize(&engine, "Foo 1.0");

  LottiePrivate p;
  LottiePrivateDocument doc(&p);

  {
    QJSValue array = doc.getElementsByClassName("foo");
    QVERIFY(array.isArray());
    QCOMPARE(array.property("length").toInt(), 0);
  }
}

static TestDocument s_testDocument;
