/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testcommandlineparser.h"

#include <string.h>

#include "command.h"
#include "commandlineparser.h"

namespace {
bool s_executed = false;

class CommandDummy final : public Command {
 public:
  explicit CommandDummy(QObject* parent)
      : Command(parent, "ui", "dummy dummy") {}

  int run(QStringList& tokens) override {
    s_executed = true;

    CommandLineParser clp;
    QList<CommandLineParser::Option*> options;
    return clp.parse(tokens, options, false);
  }
};

Command::RegistrationProxy<CommandDummy> s_commandDummy;

}  // namespace

void TestCommandLineParser::basic_data() {
  QTest::addColumn<QStringList>("args");
  QTest::addColumn<int>("result");
  QTest::addColumn<bool>("uiExecuted");

  QTest::addRow("-h") << QStringList{"something", "-h"} << 0 << false;
  QTest::addRow("--help") << QStringList{"something", "--help"} << 0 << false;
  QTest::addRow("-v") << QStringList{"something", "-v"} << 0 << false;
  QTest::addRow("--version")
      << QStringList{"something", "--version"} << 0 << false;
  QTest::addRow("invalid long option")
      << QStringList{"something", "--foo"} << 1 << false;
  QTest::addRow("invalid short option")
      << QStringList{"something", "-f"} << 1 << false;
  QTest::addRow("ui implicit") << QStringList{"something"} << 0 << true;
  QTest::addRow("ui explicit") << QStringList{"something", "ui"} << 0 << true;
  QTest::addRow("invalid command")
      << QStringList{"something", "foo"} << 1 << false;
  QTest::addRow("ui plus -h")
      << QStringList{"something", "-h", "ui"} << 0 << false;
  QTest::addRow("ui -h") << QStringList{"something", "ui", "-h"} << 1 << true;
}
void TestCommandLineParser::basic() {
  s_executed = false;

  QFETCH(QStringList, args);

  int argc = args.length();
  char** argv = (char**)malloc(sizeof(char*) * argc);

#ifndef _MSC_VER
#  define _strdup strdup
  // strdup is depricated with msvc? and we shall use
  // _strdup. So let's define that for the other compilers too.
#endif
  for (int i = 0; i < args.length(); ++i) {
    argv[i] = _strdup(args[i].toLocal8Bit().data());
  }

  QFETCH(int, result);

  CommandLineParser clp;
  QCOMPARE(clp.parse(argc, argv), result);
  QCOMPARE(clp.argc(), argc);
  QCOMPARE(clp.argv(), argv);

  QFETCH(bool, uiExecuted);
  QCOMPARE(s_executed, uiExecuted);
}

static TestCommandLineParser s_testCommandLineParser;
