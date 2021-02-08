/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcaptiveportallookup.h"
#include "captiveportal/captiveportal.h"
#include "leakdetector.h"
#include "logger.h"
#include "mozillavpn.h"
#include "networkrequest.h"

namespace {
Logger logger(LOG_NETWORKING, "TaskCaptivePortalLookup");
}

TaskCaptivePortalLookup::TaskCaptivePortalLookup()
    : Task("TaskCaptivePortalLookup") {
  MVPN_COUNT_CTOR(TaskCaptivePortalLookup);
}

TaskCaptivePortalLookup::~TaskCaptivePortalLookup() {
  MVPN_COUNT_DTOR(TaskCaptivePortalLookup);
}

void TaskCaptivePortalLookup::run(MozillaVPN* vpn) {
  logger.log() << "Resolving the captive portal detector URL";

  NetworkRequest* request = NetworkRequest::createForCaptivePortalLookup(this);
  connect(request, &NetworkRequest::requestFailed,
          [this, vpn](QNetworkReply*, QNetworkReply::NetworkError error,
                      const QByteArray&) {
            logger.log() << "Failed to obtain captive poral IPs" << error;
            vpn->errorHandle(ErrorHandler::toErrorType(error));
            emit completed();
          });

  connect(request, &NetworkRequest::requestCompleted,
          [this, vpn](QNetworkReply*, const QByteArray& data) {
            logger.log() << "Lookup completed";
            if (vpn->captivePortal()->fromJson(data)) {
              vpn->captivePortal()->writeSettings();
            }

            emit completed();
          });
}
