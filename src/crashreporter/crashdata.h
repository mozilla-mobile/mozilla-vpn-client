/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CRASHDATA_H
#define CRASHDATA_H

#include <QObject>
/**
 * @brief A base class for platform specific crash data sent to the handler.
 * Used to pass through non-platform specific code polymorphically.
 */
class CrashData : public QObject
{
    Q_OBJECT
public:
    explicit CrashData(QObject *parent = nullptr);

signals:

};

#endif // CRASHDATA_H
