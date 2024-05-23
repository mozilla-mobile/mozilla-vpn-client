/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

class InternetChecker {
    private let logger = IOSLoggerImpl(tag: "InternetChecker")
    private let numberOfPings = 1
    private let pingTimeLimit: Double = 0.1 // 100 milliseconds

    init(pingAddress: String, callback: @escaping (Bool) -> Void) {
        do {
            let ping = try SwiftyPing(host: pingAddress, configuration: PingConfiguration(interval: 1, with: pingTimeLimit), queue: DispatchQueue.global())
            ping.observer = { (response) in
                if let error = response.error {
                    self.logger.error(message: "Ping error: \(error)")
                    // Ping did not return, presuming no internet.
                    callback(false)
                    return
                }

                self.logger.debug(message: "InternetChecker success")
                callback(response.duration < self.pingTimeLimit)
            }
            ping.targetCount = numberOfPings
            logger.info(message: "Sending ping")
            try ping.startPinging()
        } catch {
            logger.error(message: "Error when sending pings: \(error)")
            // If ping had sending/setup error, the internet could still be active. We're failing to true,
            // (meaning "active internet") - so that we err on the side of trying a silent server switch.
            callback(true)
        }
    }
}
