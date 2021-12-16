/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef IMAGEPROVIDERFACTORY_H
#define IMAGEPROVIDERFACTORY_H

#include <QQuickImageProvider>

class ImageProviderFactory final
{
public:
    ImageProviderFactory() = delete;
    static QQuickImageProvider * create(QObject *parent);
};

#endif // IMAGEPROVIDERFACTORY_H
