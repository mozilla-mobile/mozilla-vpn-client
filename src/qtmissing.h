/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef MISSING_H
#  if __has_include(<QtCore/qglobal.h>)
#    include <QtCore/qglobal.h>

/* Q_DISABLE_COPY_MOVE was added in 5.13 */
#    ifndef Q_DISABLE_COPY_MOVE
#      define Q_DISABLE_COPY_MOVE(type) Q_DISABLE_COPY(type)
#    endif

/* Qt::endl() was added in 5.14 */
#    if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
class QTextStream;
Q_CORE_EXPORT QTextStream& endl(QTextStream&);

namespace Qt {
using ::endl;
}

#    endif

#  endif  // __has_include(...)
#endif    // MISSING_H
