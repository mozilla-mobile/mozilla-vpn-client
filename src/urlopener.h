/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef URLOPENER_H
#define URLOPENER_H

class QUrl;

class UrlOpener final {
 public:
  static void open(QUrl url, bool addEmailAddress = false);
};

#endif  // URLOPENER_H
