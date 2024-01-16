/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

struct Constants {
  static let appGroupIdentifier = "group.org.mozilla.ios.Guardian"
  static let networkExtensionLogFileName = "networkextension.log"

  struct UserDefaultKeys {
    static let telemetryEnabled = "TELEMETRY_ENABLED"
    static let appChannel = "APP_CHANNEL"
    static let firstRecent = "firstRecent"
    static let secondRecent = "secondRecent"
    static let current = "current"


  }
}
