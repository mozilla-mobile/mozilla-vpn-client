/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "incrementaldecoder.h"

#include <QTextStream>

#include "leakdetector.h"

constexpr int RADIX = 36;

IncrementalDecoder::IncrementalDecoder(QObject* parent) : QObject(parent) {
  MZ_COUNT_CTOR(IncrementalDecoder);
}

IncrementalDecoder::~IncrementalDecoder() { MZ_COUNT_DTOR(IncrementalDecoder); }

IncrementalDecoder::Result IncrementalDecoder::match(QTextStream& stream,
                                                     const QString& input) {
  uint32_t index = 0;
  QString prev;

  while (!stream.atEnd()) {
    QString line = stream.readLine();
    if (line.length() == 0) {
      return DecodeFailure;
    }

    QChar ch = line.front();

    bool ok = false;
    int numShared = QString(ch).toInt(&ok, RADIX);
    if (ok == false) {
      return DecodeFailure;
    }

    if (index == 0 && numShared != 0) {
      return DecodeFailure;
    }

    QString decoded = line.remove(0, 1);
    if (numShared != 0) {
      decoded = prev.left(numShared) + decoded;
    }

    prev = decoded;
    ++index;

    if (decoded == input) {
      return MatchFound;
    }
  }

  return MatchNotFound;
}
