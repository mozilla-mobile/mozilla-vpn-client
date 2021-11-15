/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef NEBULA_H
#define NEBULA_H

#include <QQmlEngine>

class Nebula final
{
private:
    Nebula() = default;
public:
    static void Initialize(QQmlEngine * engine);
};

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define INIT_NEBULA   Q_INIT_RESOURCE(themes); \
Q_INIT_RESOURCE(components); \
Q_INIT_RESOURCE(nebula_resources); \
Q_INIT_RESOURCE(compatQt6); \
Q_INIT_RESOURCE(resourcesQt6);
#else
#define INIT_NEBULA   Q_INIT_RESOURCE(themes); \
Q_INIT_RESOURCE(components); \
Q_INIT_RESOURCE(nebula_resources); \
Q_INIT_RESOURCE(compatQt5);
#endif

#endif // NEBULA_H
