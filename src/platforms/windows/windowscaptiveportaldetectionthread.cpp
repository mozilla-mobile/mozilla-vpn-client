/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "windowscaptiveportaldetectionthread.h"
#include "leakdetector.h"
#include "logger.h"
#include "windowscommons.h"

#include <QScopeGuard>

#include <windows.h>

namespace {
Logger logger(LOG_WINDOWS, "WindowsCaptivePortalDetectionThread");
HMODULE s_tunnelLib;

typedef struct {
  const char* p;
  size_t n;
} gostring_t;

typedef int32_t WireguardTestOutsideConnectivity(gostring_t ip, gostring_t host,
                                                 gostring_t url,
                                                 gostring_t expectedTestResult);

WireguardTestOutsideConnectivity* s_captivePortalDetection = nullptr;
}  // namespace

static void tunnelLoggerFunc(int level, const char* msg) {
  Q_UNUSED(level);
  logger.log() << "tunnel.dll:" << msg;
}

static bool loadLibrary() {
  Q_ASSERT(!s_tunnelLib);

  // This process will be used by the wireguard tunnel the whole time. No need
  // to call FreeLibrary.
  s_tunnelLib = LoadLibrary(TEXT("tunnel.dll"));
  if (!s_tunnelLib) {
    WindowsCommons::windowsLog("Failed to load tunnel.dll");
    return false;
  }

  typedef void (*logFunc)(int level, const char* msg);
  typedef void WireGuardTunnelLogger(logFunc func);

  WireGuardTunnelLogger* tunnelLogger = (WireGuardTunnelLogger*)GetProcAddress(
      s_tunnelLib, "WireGuardTunnelLogger");
  if (!tunnelLogger) {
    WindowsCommons::windowsLog("Failed to get WireGuardTunnelLogger function");
    return false;
  }

  tunnelLogger(tunnelLoggerFunc);

  s_captivePortalDetection = (WireguardTestOutsideConnectivity*)GetProcAddress(
      s_tunnelLib, "WireguardTestOutsideConnectivity");
  if (!s_captivePortalDetection) {
    WindowsCommons::windowsLog(
        "Failed to get WireguardTestOutsideConnectivity function");
    return false;
  }

  return true;
}

WindowsCaptivePortalDetectionThread::WindowsCaptivePortalDetectionThread(
    QObject* parent)
    : QObject(parent){MVPN_COUNT_CTOR(WindowsCaptivePortalDetectionThread)}

      WindowsCaptivePortalDetectionThread::
          ~WindowsCaptivePortalDetectionThread() {
  MVPN_COUNT_DTOR(WindowsCaptivePortalDetectionThread)
}

void WindowsCaptivePortalDetectionThread::startWorker(
    const QString& ip, const QString& host, const QString& url,
    const QString& expectedResult) {
  logger.log() << "starting the captive portal detection";

  auto guard = qScopeGuard([&] { emit detectionCompleted(false); });

  if (!s_tunnelLib && !loadLibrary()) {
    logger.log() << "Failed to load the library";
    return;
  }

  Q_ASSERT(s_tunnelLib);

  if (!s_captivePortalDetection) {
    logger.log() << "No captive portal function found";
    return;
  }

  QByteArray ipArray(ip.toLocal8Bit());
  gostring_t ipGo{ipArray.constData(), ipArray.length()};

  QByteArray hostArray(host.toLocal8Bit());
  gostring_t hostGo{hostArray.constData(), hostArray.length()};

  QByteArray urlArray(url.toLocal8Bit());
  gostring_t urlGo{urlArray.constData(), hostArray.length()};

  QByteArray expectedResutArray(expectedResult.toLocal8Bit());
  gostring_t expectedResultGo{expectedResutArray.constData(),
                              expectedResutArray.length()};

  int32_t result =
      s_captivePortalDetection(ipGo, hostGo, urlGo, expectedResultGo);
  switch (result) {
    case -1:
      logger.log() << "Failed to detect the captive portal";
      return;

    case 0:
      logger.log() << "Captive portal detected!";
      guard.dismiss();
      emit detectionCompleted(true);
      return;

    case 1:
      logger.log() << "No captive portal detected";
      return;

    default:
      logger.log() << "Invalid return value:" << result;
      return;
  }
}
