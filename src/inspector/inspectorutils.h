/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTORUTILS_H
#define INSPECTORUTILS_H

#include <QObject>

class InspectorUtils final {
 public:
  static QObject* findObject(const QString& name);
};

#endif  // INSPECTORUTILS_H
