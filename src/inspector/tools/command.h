/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef INSPECTOR_COMMAND_H
#  define INSPECTOR_COMMAND_H

#include <QString>
#include <QByteArray>

namespace InspectorTools {
	struct Command {
	  QString m_commandName;
	  QString m_commandDescription;
	  int32_t m_arguments;
	  std::function<QJsonObject(const QList<QByteArray>&)> m_callback;
	};

}

#endif 