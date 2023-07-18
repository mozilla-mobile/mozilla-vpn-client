/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SENTRYADAPTER_H
#define SENTRYADAPTER_H

#ifdef SENTRY_ENABLED
#  include <sentry.h>
#else
// Define those types for testing.
typedef void* sentry_ucontext_t;
typedef void* sentry_value_t;
typedef void* sentry_envelope_t;
#endif

#include <QApplication>
#include <QObject>

class SentryAdapter final : public QObject {
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(SentryAdapter)

 public:
  ~SentryAdapter();
  static SentryAdapter* instance();

  /**
   * @brief Inits Sentry.
   *
   * This is a no-op if the client is in production mode.
   */
  void init();

  /**
   * @brief Checks if Sentry is initalized
   * 
   * @return true - Sentry is ready to report crashes
   * @return false - Sentry failed to Init
   */
  inline bool const ready(){ return m_initialized; }

  /**
   * @brief Sends an "Issue" report to Sentry
   *
   * @param category - "Category" of the error, any String is valid.
   * @param message - Additional message content.
   * @param attachStackTrace - If true a stacktrace for later debugging will be
   * attached.
   */
  void report(const QString& category, const QString& message,
              bool attachStackTrace = false);

  /**
   * @brief Event Slot for when a log-line is added.
   *
   * The logline will be added as a Sentry-Breadrumb, so that
   * the next "report" or crash will have the last few
   * loglines available
   *
   * @param line - UTF-8 encoded bytes of the logline.
   */
  Q_SLOT void onLoglineAdded(const QByteArray& line);

  /**
   * @brief Event Slot for when the client is about to Shut down
   *
   * This will call sentry to wrap up - this might cause new network requests
   * or drisk writes.
   * After calling this, any call to sentry is UB.
   */
  Q_SLOT void onBeforeShutdown();

  /**
   * @brief Callback for when sentry's backend recieved a crash.
   *
   * In this function we can decide to either send, discard the crash
   * and additonally "scrub" the minidump of data, if wanted.
   *
   * @param uctx provides the user-space context of the crash
   * @param event used the same way as in `before_send`
   * @param closure user-data that you can provide at configuration time
   * (we'dont.)
   * @return either the @param event or null_sentry_value , if the crash event
   * should not be recorded.
   */
  static sentry_value_t onCrash(const sentry_ucontext_t* uctx,
                                sentry_value_t event, void* closure);

  /**
   * @brief Send's a Sentry Event "envelope" to the Sentry endpoint.
   *
   * Will be used if NONE_TRANSPORT is enabled in cmake.
   * Will create a Task in the TaskSheudler to send that.
   *
   * @param envelope Envelope to be sent to sentry.
   * The transport takes ownership of the `envelope`, and must free it once it
   * is done.
   * @param state
   * If the transport requires state, such as an HTTP
   * client object or request queue, it can be specified in the `state`
   * parameter when configuring the transport. It will be passed as second
   * argument to this function. We are not using that.
   *
   */
  static void transportEnvelope(sentry_envelope_t* envelope, void* state);

  enum UserConsentResult {
    Pending = -1,   // The User has to be asked
    Forbidden = 0,  // The User not given consent
    Allowed = 1     // The User gave consent
  };
  Q_ENUM(UserConsentResult)

  /**
   * @brief Checks if the user gave consent to upload crashes
   *
   * @return UserConsentResult - Allowed/Forbidden in case we have gotten
   * consent Can return "Pending", in which case a prompt has been fired to the
   * UI to ask the user.
   * - listen for userConsentChanged to be notified when that is done
   */
  UserConsentResult hasCrashUploadConsent() const;

  /**
   * @brief Allows Crash Reporting for this Session
   */
  Q_INVOKABLE void allowCrashReporting();
  /**
   * @brief Disables Crash Reporting for this Session
   */
  Q_INVOKABLE void declineCrashReporting();

  /**
   * @brief Helper Function, will print the current
   * stacktrace of the QJSEngine.
   * @param description - Description of the Stacktrace.
   */
  Q_INVOKABLE static void captureQMLStacktrace(const char* description);

 signals:
  /**
   * @brief Signal that is fired whenever the consent changed, note: can sill be
   * Pending.
   *
   * @return bool
   */
  void userConsentChanged();

  /**
   * @brief Signal that is fired when the CrashReport screen needs to be shown
   */
  void needsCrashReportScreen() const;

 private:
  bool m_initialized = false;
  UserConsentResult m_userConsent = UserConsentResult::Pending;
  SentryAdapter();
};
#endif  // SENTRYADAPTER_H
