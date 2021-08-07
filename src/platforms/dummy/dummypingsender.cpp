/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "dummypingsender.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_NETWORKING, "DummyPingSender");
}

DummyPingSender::DummyPingSender(const QString& source, QObject* parent)
    : PingSender(parent) {
  MVPN_COUNT_CTOR(DummyPingSender);
  Q_UNUSED(source);
}

DummyPingSender::~DummyPingSender() { MVPN_COUNT_DTOR(DummyPingSender); }

void DummyPingSender::sendPing(const QString& dest, quint16 sequence) {
  logger.debug() << "Dummy ping to:" << dest;
  emit recvPing(sequence);
}
