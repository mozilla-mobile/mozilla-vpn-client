/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_VPN_TESTTASKSENTRY_H
#define MOZILLA_VPN_TESTTASKSENTRY_H

#include "helper.h"

class TestTaskSentry final : public TestHelper {
  Q_OBJECT

 private slots:

  // All The invalid ones
  // All those tests are expected to result in
  // ContentType::Unknown
  void emptyEnvelope();
  void emptyHeaderEnvelope();
  void envelopeWithJustAnAttachment();
  void wrongOrderEnvelope();

  /**
   * @brief Tests examples of valid non-crash-report envelopes
   * See https://develop.sentry.dev/sdk/envelopes/#full-examples
   * the type of parsing should be Ping
   */
  void valid2EmptyAttachments();
  void validWithAttachments();
  void validImplicitLength();
  void noHeaderButSession();
  void levelErrorIsFine();

  /**
   * @brief Tests containing crashdata
   *
   */
  void detectMinidump();
  void sniffMinidump();
  void detectAttachmentMinidump();
  void detectFatalErrors();
};

#endif  // MOZILLA_VPN_TESTTASKSENTRY_H
