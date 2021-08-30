/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INCREMENTALDECODER_H
#define INCREMENTALDECODER_H

#include <QObject>

class QTextStream;

// This class implements the decoder part of this
// https://en.wikipedia.org/wiki/Incremental_encoding

class IncrementalDecoder final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(IncrementalDecoder)

 public:
  explicit IncrementalDecoder(QObject* parent);
  ~IncrementalDecoder();

  enum Result {
    DecodeFailure,
    MatchFound,
    MatchNotFound,
  };
  Q_ENUM(Result)

  Result match(QTextStream& stream, const QString& input);
};

#endif  // INCREMENTALDECODER_H
