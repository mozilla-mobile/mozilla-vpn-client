/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef ADJUSTFILTERING_H
#define ADJUSTFILTERING_H

#include <QUrl>
#include <QUrlQuery>

class AdjustFiltering final {
 public:
  // Public for unit-tests. Returns a new list of parameters filtering out what
  // we don't like.
  static QUrlQuery filterParameters(QUrlQuery& parameters,
                                    QStringList& unknownParameters);
};

#endif  // ADJUSTFILTERING_H
