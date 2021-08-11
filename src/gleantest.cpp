/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gleantest.h"
#include "leakdetector.h"
#include "logger.h"

namespace {
Logger logger(LOG_MAIN, "GleanTest");

GleanTest* s_instance = nullptr;
}  // namespace

GleanTest::GleanTest() {
  MVPN_COUNT_CTOR(GleanTest);

  Q_ASSERT(!s_instance);
  s_instance = this;
}

GleanTest::~GleanTest() {
  MVPN_COUNT_DTOR(GleanTest);

  Q_ASSERT(s_instance == this);
  s_instance = nullptr;
}

// static
GleanTest* GleanTest::instance() {
  Q_ASSERT(s_instance);
  return s_instance;
}

void GleanTest::requestDone(const QByteArray& url, const QByteArray& data) {
  logger.debug() << "Glean request stored";

  m_lastUrl = url;
  m_lastData = data;
}

void GleanTest::reset() {
  m_lastUrl.clear();
  m_lastData.clear();
}
