/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ANDROIDAUTHENTICATIONLISTENER_H
#define ANDROIDAUTHENTICATIONLISTENER_H

#include "tasks/authenticate/authenticationlistener.h"

class AndroidAuthenticationListener final : public AuthenticationListener
{
public:
    AndroidAuthenticationListener(QObject *parent);

    void start(MozillaVPN *vpn, QUrl &url, QUrlQuery &query) override;
};

#endif // ANDROIDAUTHENTICATIONLISTENER_H
