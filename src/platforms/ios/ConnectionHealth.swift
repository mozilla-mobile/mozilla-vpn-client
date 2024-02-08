/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

class ConnectionHealth {
    private let logger = IOSLoggerImpl(tag: "ConnectionHealthSwift")

    // Timer should fire between 30 and 32 seconds.
    private let checkTime = TimeInterval(30) // 30 seconds
    private let toleranceTime = TimeInterval(2) // 2 seconds

    private var timer: Timer?

    func start() {
        // There is no restart on a Timer, so we need to recreate it each time
       timer = Timer.scheduledTimer(timeInterval: checkTime, target: self, selector: #selector(runHealthChecks), userInfo: nil, repeats: true)
       // Giving timer some tolerance improves performance.
       timer?.tolerance = TimeInterval(toleranceTime)
    }

    func stop() {
        guard let confirmedTimer = timer else {
            logger.error(message: "No timer found when stopping ConnectionHealth")
            return
        }
        confirmedTimer.invalidate()
        timer = nil
    }

    @objc private func runHealthChecks() {

    }
}
