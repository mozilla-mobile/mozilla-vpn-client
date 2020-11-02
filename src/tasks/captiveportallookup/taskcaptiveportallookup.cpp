/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "taskcaptiveportallookup.h"
#include "logger.h"
#include "captiveportal/captiveportallookup.h"

namespace {
Logger logger(LOG_NETWORKING, "TaskCaptivePortalLookup");
}

void TaskCaptivePortalLookup::run(MozillaVPN *vpn)
{
    Q_UNUSED(vpn);

    logger.log() << "Resolving the captive portal detector URL";

    CaptivePortalLookup* cpl = new CaptivePortalLookup(this);
    connect(cpl, &CaptivePortalLookup::completed, this, &TaskCaptivePortalLookup::completed);
    cpl->start();
}
