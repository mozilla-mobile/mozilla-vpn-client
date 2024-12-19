/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "killernetwork.h"

#include <qobject.h>

#include "platforms/windows/windowsutils.h"

namespace Intervention {

const QString KillerNetwork::id = "intel.killernetwork";

bool KillerNetwork::systemAffected() {
  const bool isKillerInstalledAndRunning =
      WindowsUtils::getServiceStatus("Killer Network Service");
  return isKillerInstalledAndRunning;
}

}  // namespace Intervention