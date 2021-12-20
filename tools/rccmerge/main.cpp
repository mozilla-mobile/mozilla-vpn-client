/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "../../src/resourceloaderutils.h"

#include <QFile>
#include <QFileInfo>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QTextStream>

bool writeData(QFile* file, const QByteArray& data) {
  uint32_t length = (uint32_t)data.length();
  char* rawLength = reinterpret_cast<char*>(&length);

  return file->write(rawLength, sizeof(uint32_t)) == sizeof(uint32_t) &&
         file->write(data.constData(), length) == length;
}

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);

  QCoreApplication::setApplicationName("Mozilla VPN - RCC merge");
  QCoreApplication::setApplicationVersion(APP_VERSION);

  QCommandLineParser parser;
  parser.setApplicationDescription("Mozilla VPN - RCC merge");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption outputFileOption(
      QStringList() << "o"
                    << "output",
      "Set the output file. If not set, stderr will be used.", "output file");
  parser.addOption(outputFileOption);

  QCommandLineOption forceOption(QStringList() << "f"
                                               << "force",
                                 "Overwrite existing output file.");
  parser.addOption(forceOption);

  QCommandLineOption minVersionOption(QStringList() << "m"
                                                    << "min",
                                      "Min client version", "version");
  parser.addOption(minVersionOption);

  QCommandLineOption maxVersionOption(QStringList() << "M"
                                                    << "max",
                                      "Max client version", "version");
  parser.addOption(maxVersionOption);

  parser.addPositionalArgument("input", "List of input files to merge",
                               "[inputs...]");

  QCommandLineOption restartRequiredOption(QStringList() << "r"
                                                         << "restart",
                                           "Restart required after update");
  parser.addOption(restartRequiredOption);

  parser.process(app);

  QTextStream output(stdout);
  output << QCoreApplication::applicationName() << " "
         << QCoreApplication::applicationVersion() << Qt::endl
         << Qt::endl;

  if (!parser.isSet(minVersionOption) || !parser.isSet(maxVersionOption)) {
    output << "Min/Max client version params are required." << Qt::endl;
    return 1;
  }

  const QStringList args = parser.positionalArguments();
  if (args.isEmpty()) {
    output << "Please, at least 1 input file is required." << Qt::endl;
    return 1;
  }

  QString outputFileName = parser.value(outputFileOption);
  if (parser.isSet(outputFileOption) && QFile::exists(outputFileName) &&
      !parser.isSet(forceOption)) {
    output << outputFileName << " exists. Please remove it." << Qt::endl;
    return 1;
  }

  QFile* outputFile = nullptr;
  if (parser.isSet(outputFileOption)) {
    output << ". Opening the output file " << outputFileName << "... ";
    outputFile = new QFile(outputFileName, &app);
    if (!outputFile->open(QFile::WriteOnly)) {
      output << "Failed to open " << args[0] << Qt::endl;
      return 1;
    }
  } else {
    output << ". Using stderr... ";
    outputFile = new QFile(&app);
    if (!outputFile->open(stderr, QFile::WriteOnly)) {
      output << "Failed to open " << args[0] << Qt::endl;
      return 1;
    }
  }
  output << "done." << Qt::endl;

  output << ". Writing the header... ";
  if (outputFile->write("MVPN", 4) != 4 ||
      outputFile->write(QByteArray(1, 0x01)) != 1) {
    output << "Failed.";
    return 1;
  }
  output << "done." << Qt::endl;

  QJsonArray resourceArray;

  for (int i = 1; i < args.length(); ++i) {
    QFile inputFile(args[i]);
    output << ". File " << inputFile.fileName() << Qt::endl;

    output << "  - Opening " << inputFile.fileName() << "... ";
    if (!inputFile.open(QFile::ReadOnly)) {
      output << "Failed to open " << inputFile.fileName() << Qt::endl;
      return 1;
    }
    output << "done." << Qt::endl;

    QString fileName = QFileInfo(inputFile.fileName()).fileName();
    output << "  - Validating " << fileName << "... ";
    if (!ResourceLoaderUtils::isFileNameValid(fileName)) {
      output << "Invalid filename." << Qt::endl;
      return 1;
    }
    output << "done." << Qt::endl;

    output << "  - Writing filename... ";
    if (!writeData(outputFile, fileName.toLocal8Bit())) {
      output << "Failed.";
      return 1;
    }
    output << "done." << Qt::endl;

    output << "  - Writing content... ";
    const QByteArray content = inputFile.readAll();
    if (!writeData(outputFile, content)) {
      output << "Failed.";
      return 1;
    }
    output << "done." << Qt::endl;

    output << "  - Computing sha256... ";
    QJsonObject obj;
    obj["fileName"] = fileName;
    obj["sha256"] =
        QString(QCryptographicHash::hash(content, QCryptographicHash::Sha256)
                    .toBase64(QByteArray::Base64Encoding));
    obj["size"] = content.length();
    resourceArray.append(obj);
    output << "done." << Qt::endl;
  }

  output << ". Manifest generation... ";
  QJsonObject manifestObj;
  manifestObj["version"] = 1;
  manifestObj["resources"] = resourceArray;
  manifestObj["minVersion"] = parser.value(minVersionOption);
  manifestObj["maxVersion"] = parser.value(maxVersionOption);
  manifestObj["restartRequired"] = parser.isSet(restartRequiredOption);
  QByteArray manifestJson = QJsonDocument(manifestObj).toJson();

  // TODO: signature of the manifest

  if (!writeData(outputFile, "manifest.json") ||
      !writeData(outputFile, manifestJson)) {
    output << "Failed.";
    return 1;
  }
  output << "done." << Qt::endl;

  return 0;
}
