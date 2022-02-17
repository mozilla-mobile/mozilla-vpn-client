/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "connectionbenchmark.h"
#include "leakdetector.h"
#include "logger.h"
#include "pinghelper.h"

namespace {
Logger logger(LOG_NETWORKING, "ConnectionBenchmark");
}

ConnectionBenchmark::ConnectionBenchmark() {
  MVPN_COUNT_CTOR(ConnectionBenchmark);
}

ConnectionBenchmark::~ConnectionBenchmark() {
  MVPN_COUNT_DTOR(ConnectionBenchmark);
}

void ConnectionBenchmark::setState(State state) {
  logger.debug() << "set state" << state;
  m_state = state;
  emit stateChanged();
}

void ConnectionBenchmark::start() {
  logger.debug() << "start benchmark";

  setState(StateTesting);
}

void ConnectionBenchmark::stop() {
  logger.debug() << "stop benchmark";

  setState(StateInitial);
}

const quint64& ConnectionBenchmark::pingValue() {
  logger.debug() << "ping value";

  m_pingValue = 1234;

  return m_pingValue;
}
