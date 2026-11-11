/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testcommandlineparser.h"

#include <QtTest/QtTest>

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

  for (int i = 0; i < args.length(); ++i) {
    argv[i] = strdup(args[i].toLocal8Bit().data());
  }

  QFETCH(int, result);

  CommandLineParser clp;
  QCOMPARE(clp.parse(argc, argv, "ui"), result);
  QCOMPARE(clp.argc(), argc);
  QCOMPARE(clp.argv(), argv);

  QFETCH(bool, uiExecuted);
  QCOMPARE(s_executed, uiExecuted);
}

void TestCommandLineParser::valueOption_data() {
  QTest::addColumn<QStringList>("args");
  QTest::addColumn<int>("result");
  QTest::addColumn<bool>("set");
  QTest::addColumn<QString>("value");
  QTest::addColumn<QStringList>("remaining");

  QTest::addRow("not set") << QStringList{"app", "pos"} << 0 << false
                           << QString() << QStringList{"pos"};
  QTest::addRow("short") << QStringList{"app", "-o", "file"} << 0 << true
                         << "file" << QStringList{};
  QTest::addRow("long") << QStringList{"app", "--output", "file"} << 0 << true
                        << "file" << QStringList{};
  QTest::addRow("value starting with dash")
      << QStringList{"app", "-o", "-file", "pos"} << 0 << true << "-file"
      << QStringList{"pos"};
  QTest::addRow("with flag and positional")
      << QStringList{"app", "-o", "file", "-f", "pos"} << 0 << true << "file"
      << QStringList{"pos"};
  QTest::addRow("missing value") << QStringList{"app", "-o"} << 1 << true
                                 << QString() << QStringList{"-o"};
}

void TestCommandLineParser::valueOption() {
  QFETCH(QStringList, args);

  CommandLineParser::Option output("o", "output", "output file", "file");
  CommandLineParser::Option flag("f", "flag", "a flag");
  QList<CommandLineParser::Option*> options{&output, &flag};

  CommandLineParser clp;
  QFETCH(int, result);
  QCOMPARE(clp.parse(args, options, false), result);

  QFETCH(bool, set);
  QCOMPARE(output.m_set, set);

  QFETCH(QString, value);
  QCOMPARE(output.m_value, value);

  QFETCH(QStringList, remaining);
  QCOMPARE(args, remaining);
}
