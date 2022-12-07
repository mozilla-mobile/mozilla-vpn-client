/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcaptiveportallookup.h"

#include "leakdetector.h"
#include "logger.h"
#include "modules/vpn.h"
#include "networkrequest.h"

namespace {
Logger logger("TaskCaptivePortalLookup");
}

TaskCaptivePortalLookup::TaskCaptivePortalLookup(
    ErrorHandler::ErrorPropagationPolicy errorPropagationPolicy)
    : Task("TaskCaptivePortalLookup"),
      m_errorPropagationPolicy(errorPropagationPolicy) {
  MVPN_COUNT_CTOR(TaskCaptivePortalLookup);
}

TaskCaptivePortalLookup::~TaskCaptivePortalLookup() {
  MVPN_COUNT_DTOR(TaskCaptivePortalLookup);
}

void TaskCaptivePortalLookup::run() {
  logger.debug() << "Resolving the captive portal detector URL";

  NetworkRequest* request = NetworkRequest::createForCaptivePortalLookup(this);
  connect(request, &NetworkRequest::requestFailed, this,
          [this](QNetworkReply::NetworkError error, const QByteArray&) {
            if (m_cancelled) {
              return;
            }
            logger.error() << "Failed to obtain captive portal IPs" << error;
            REPORTNETWORKERROR(error, m_errorPropagationPolicy, name());
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted, this,
          [this](const QByteArray& data) {
            logger.debug() << "Lookup completed";

            ModuleVPN* vpn = ModuleVPN::instance();
            if (vpn->captivePortal()->fromJson(data)) {
              vpn->captivePortal()->writeSettings();
            }

            emit completed();
          });
}
