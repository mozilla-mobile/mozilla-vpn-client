/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

// Imports built-in plugins used by the VPN. qmlimportscanner extracts these
// import statements and generates a list of libraries that supply the built-in
// plugins to the statically linked Qt on Windows.

import QtQml.Modules 2.2
import QtQuick 2.5
import QtQuick.Controls 2.15
import QtQuick.Layout 1.14
import QtQuick.Windows 2.12
import Qt5Compat.GraphicalEffects
import QtGraphicalEffects 1.0
import Qt.labs.qmlmodels 1.0

Item {}
