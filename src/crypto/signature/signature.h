/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <QByteArray>

namespace Signature {
bool verify(const QByteArray& publicKey, const QByteArray& content,
            const QByteArray& signature);

bool verifyInternal(const QByteArray& publicKey, const QByteArray& content,
                    const QByteArray& signature);
};  // namespace Signature

#endif  // SIGNATURE_H
