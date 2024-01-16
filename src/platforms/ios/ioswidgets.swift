/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation

public class IOSWidgetsImpl : NSObject {
    
    @objc func saveFirstRecent(data: String) {
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        defaults!.set(data, forKey: Constants.UserDefaultKeys.firstRecent)
        defaults!.synchronize()
    }
    
    func retrieveFirstRecent() -> String {
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        return defaults!.string(forKey: Constants.UserDefaultKeys.firstRecent)!
    }
    
    @objc func saveSecondRecent(data: String) {
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        defaults!.set(data, forKey: Constants.UserDefaultKeys.secondRecent)
        defaults!.synchronize()
    }
    
    func retrieveSecondRecent() -> String {
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        return defaults!.string(forKey: Constants.UserDefaultKeys.secondRecent)!
    }
    
    @objc func saveCurrent(data: String) {
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        defaults!.set(data, forKey: Constants.UserDefaultKeys.current)
        defaults!.synchronize()
    }
    
    func retrieveCurrent() -> String {
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        return defaults!.string(forKey: Constants.UserDefaultKeys.current)!
    }
}
