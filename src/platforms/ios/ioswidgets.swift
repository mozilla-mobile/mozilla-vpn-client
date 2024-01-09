/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation

public class IOSWidgetsImpl : NSObject {
    
    @objc func saveToUserDefaults() {
        print("Saving to user defaults")
        
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        defaults!.set("xys", forKey: Constants.UserDefaultKeys.test)
        defaults!.synchronize()
}
    
    func retrieveFromUserDefaults() -> String {
        print("Retrieving from user defaults")
        
        let defaults = UserDefaults(suiteName: Constants.appGroupIdentifier)
        return defaults!.string(forKey: Constants.UserDefaultKeys.test)!
    }
}
