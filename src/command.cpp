/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "command.h"

QVector<Command *> Command::s_commands;

Command::Command(const QString &name, const QString &description)
    : m_name(name), m_description(description)
{
    s_commands.append(this);
}
