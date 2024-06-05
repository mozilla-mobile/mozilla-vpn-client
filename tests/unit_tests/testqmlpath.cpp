/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testqmlpath.h"

#include <QQmlApplicationEngine>
#include <QQuickItem>

#include "qmlpath.h"

void TestQmlPath::parse_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("result");

  // Testing the path parser
  QTest::addRow("empty") << "" << false;
  QTest::addRow("root") << "/" << true;
  QTest::addRow("search without key") << "//" << false;
  QTest::addRow("key") << "abc" << true;
  QTest::addRow("root+key") << "/abc" << true;
  QTest::addRow("search key") << "//abc" << true;
  QTest::addRow("two blocks") << "/abc/cba" << true;
  QTest::addRow("two blocks + search") << "/abc//cba" << true;
  QTest::addRow("two blocks + unfinished search") << "/abc/cba//" << false;
  QTest::addRow("two blocks + extra slash") << "/abc/cba/" << true;

  // Testing the array selection
  QTest::addRow("non-terminated array 1") << "/abc[" << false;
  QTest::addRow("non-terminated array 2") << "/abc[123" << false;
  QTest::addRow("root element + index") << "/[123]" << true;
  QTest::addRow("item + index") << "/abc[123]" << true;
  QTest::addRow("search + index") << "//abc[123]" << true;
  QTest::addRow("negative index") << "/abc[-1]" << false;
  QTest::addRow("complex ranges") << "/[123]/b[1]/c//d/e[0]/f/[6]" << true;

  // Property
  QTest::addRow("non-terminated property 1") << "/abc{" << false;
  QTest::addRow("non-terminated property 2") << "/abc{foo" << false;
  QTest::addRow("empty property 1") << "/abc{=}" << false;
  QTest::addRow("empty property 2") << "/abc{=b}" << false;
  QTest::addRow("terminated property without value") << "/abc{foo}" << true;
  QTest::addRow("terminated property with empty value") << "/abc{foo=}" << true;
  QTest::addRow("terminated property with value") << "/abc{foo=bar}" << true;
  QTest::addRow("item + property") << "/abc{foo}" << true;
  QTest::addRow("item + property + value") << "/abc{foo=bar}" << true;
  QTest::addRow("search + property") << "//abc{foo}" << true;
  QTest::addRow("search + property + value") << "//abc{foo=bar}" << true;

  // Property + array selection
  QTest::addRow("mix 1") << "/[123][1]" << true;
  QTest::addRow("mix 2") << "/{a}{b}" << true;
  QTest::addRow("mix 3") << "/{a}[1]{b}" << true;
  QTest::addRow("mix 4") << "/{a}[1]{b}" << true;
  QTest::addRow("mix 5") << "/{a}[1]{b}[2]" << true;
  QTest::addRow("mix 6") << "/[123]{a}[2]/b[1]{b=c}/c//d{d}/e[0]/f/[6]{e=f}[5]"
                         << true;
}

void TestQmlPath::parse() {
  QFETCH(QString, input);
  QmlPath qmlPath(input);

  QFETCH(bool, result);
  QCOMPARE(qmlPath.isValid(), result);
}

void TestQmlPath::evaluate_data() {
  QTest::addColumn<QString>("input");
  QTest::addColumn<bool>("result");
  QTest::addColumn<QString>("name");

  QTest::addRow("select the root element") << "/" << true << "abc";
  QTest::addRow("select the root element by name") << "/abc" << true << "abc";
  QTest::addRow("select an invalid root element by name")
      << "/invalid" << false;
  QTest::addRow("select the nested element without name")
      << "/abc/" << true << "def";
  QTest::addRow("select the nested element by name")
      << "/abc/def" << true << "def";
  QTest::addRow("select the nested-nested element by name")
      << "/abc/def/ghi" << true << "ghi";
  QTest::addRow("select an invalid nested element by name")
      << "/abc/invalid" << false;
  QTest::addRow("select an invalid nested-nested element by name")
      << "/abc/def/invalid" << false;
  QTest::addRow("search the root element") << "//abc" << true << "abc";
  QTest::addRow("search an invalid element") << "//invalid" << false;
  QTest::addRow("search the nested element") << "//def" << true << "def";
  QTest::addRow("search the nested-nested element") << "//ghi" << true << "ghi";
  QTest::addRow("search the nested element, then the first element")
      << "//def/" << true << "ghi";
  QTest::addRow("search the nested-nested element, plus the next item")
      << "//ghi/" << false;
  QTest::addRow("search the nested element from an object")
      << "/abc//def" << true << "def";
  QTest::addRow("search the nested-nested element from an object")
      << "/abc//ghi" << true << "ghi";
  QTest::addRow("search an invalid element from an object")
      << "/abc//invalid" << false;
  QTest::addRow("search from an object, then the first element")
      << "/abc//def/" << true << "ghi";

  // Repeater crazyness: the objects are stored in the parent
  QTest::addRow("select for apple in repeater")
      << "/abc/apple" << true << "apple";
  QTest::addRow("search for apple in repeater") << "//apple" << true << "apple";

  /*

    These tests break when run with statically compiled
    Qt6.6.3 on mac.

    See: https://mozilla-hub.atlassian.net/browse/VPN-6423

    // objects with contentItem property (lists)
    QTest::addRow("select item in lists")
        << "/abc/list/artichoke" << true << "artichoke";
    QTest::addRow("search for item a lists")
        << "//artichoke" << true << "artichoke";
    QTest::addRow("search for list, then item")
        << "//list/artichoke" << true << "artichoke";

  */

  // Indexing
  QTest::addRow("root index") << "/[0]" << true << "abc";
  QTest::addRow("root out of range") << "/[1]" << false;
  QTest::addRow("select item in range") << "/abc[0]" << true << "abc";
  QTest::addRow("select item out of range") << "/abc[1]" << false;
  QTest::addRow("select item in range (0)")
      << "/abc/rangeA[0]" << true << "rangeA";
  QTest::addRow("select item in range (1)")
      << "/abc/rangeA[1]" << true << "rangeA";
  QTest::addRow("search for item in range") << "//ghi[0]" << true << "ghi";
  QTest::addRow("search for item out of range") << "/ghi[1]" << false;
  QTest::addRow("search for item in range (0)")
      << "//rangeB[0]" << true << "rangeB";
  QTest::addRow("search for item in range (1)")
      << "//rangeB[1]" << true << "rangeB";
  QTest::addRow("search for item in range, then something")
      << "//rangeB[1]/foo" << true << "foo";

  // Property
  QTest::addRow("filter by class name (component)")
      << "//loader/{className=FooBar}" << true << "";
  QTest::addRow("filter by class name (QML Item)")
      << "/abc{className=Item}" << true << "abc";
  QTest::addRow("filter by property - no value")
      << "/abc{pBool}" << true << "abc";
  QTest::addRow("filter by property - invalid") << "/abc{p2}" << false;
  QTest::addRow("filter by property with value")
      << "/abc{pBool=true}" << true << "abc";
  QTest::addRow("filter by property with value - not match")
      << "/abc{pBool=false}" << false;
  QTest::addRow("filter by property (string)")
      << "/abc{pString=ok}" << true << "abc";
  QTest::addRow("filter by property (int)") << "/abc{pInt=42}" << true << "abc";
  QTest::addRow("nested with property") << "//foo{p=42}/bar" << true << "bar";

  // Property + index
  QTest::addRow("first index, then property")
      << "//filterA[1]{p1=B}" << true << "filterA";
  QTest::addRow("first index, then property, plus extra indexing")
      << "//filterA[1]{p1=B}[0]" << true << "filterA";
  QTest::addRow("first property, then index")
      << "//filterA{p1=B}[0]" << true << "filterA";
  QTest::addRow("search by property")
      << "//filters/{p1=B}[1]/filterB" << true << "filterB";
}

void TestQmlPath::evaluate() {
  QQmlApplicationEngine engine("qrc:a.qml");

  QFETCH(QString, input);
  QmlPath qmlPath(input);

  QVERIFY(qmlPath.isValid());

  QFETCH(bool, result);
  QQuickItem* output = qmlPath.evaluate(&engine);
  QCOMPARE(!!output, result);

  if (output) {
    QFETCH(QString, name);
    QCOMPARE(output->objectName(), name);
  }
}

static TestQmlPath s_testQmlPath;
