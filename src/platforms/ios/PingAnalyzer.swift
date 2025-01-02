/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

class PingAnalyzer {
    private let logger = IOSLoggerImpl(tag: "PingAnalyzer")

    private let numberOfPings = 20
    private let pingSendTimespan: Double = 5.0 // send numberOfPings over 5 seconds

    private var checkTime: Double {
        return pingSendTimespan + pingTimeoutSeconds
    }
    private var timer: Timer = Timer()

    // In seconds, how often to send a ping
    private var pingSendTimeSeconds: Double {
        return pingSendTimespan / Double(numberOfPings)
    }

    private var returnedPings = 0

    private let pingTimeoutSeconds: Double = 0.5
    private let pingLossUnstableThreshold: Double = 0.65 // 13 of 20 pings
    private let pingLossNoSignalThreshold: Double = 1.0 // all pings

    private let callback: (ConnectionHealth.ConnectionStability?, Error?) -> Void

    init(pingAddress: String, callback: @escaping (ConnectionHealth.ConnectionStability?) -> Void) {
        self.callback = callback

        do {
            let ping = try SwiftyPing(host: pingAddress, configuration: PingConfiguration(interval: pingSendTimeSeconds, with: pingTimeoutSeconds), queue: DispatchQueue.global())
            ping.observer = { (response) in
                if let error = response.error {
                    self.logger.error(message: "Ping error: \(error)")
                    return
                }

                if (response.duration < self.pingTimeoutSeconds) {
                    self.returnedPings += 1
                }

                if self.returnedPings == self.numberOfPings {
                    self.timer.invalidate()
                    self.calculateStability()
                }
            }
            ping.targetCount = numberOfPings
            logger.info(message: "Sending pings")
            try ping.startPinging()
            timer = Timer.scheduledTimer(timeInterval: TimeInterval(checkTime), target: self, selector: #selector(calculateStability), userInfo: nil, repeats: false)
        } catch {
            logger.error(message: "Error when sending pings: \(error)")
            callback(nil, error)
        }
    }

    // This will be called when all pings return or the timer completes, whichever comes first.
    @objc func calculateStability() {
        let packetLoss = numberOfPings - returnedPings
        let packetLossPercent = Double(packetLoss) / Double(numberOfPings)

        logger.error(message: "Calculated stability: Packet loss \(packetLoss)")

        // If any pings take too long to return or
        if (packetLossPercent >= pingLossNoSignalThreshold) {
            callback(.noSignal, nil)
        } else if (packetLossPercent >= pingLossUnstableThreshold) {
            callback(.unstable, nil)
        } else {
            callback(.stable, nil)
        }
    }
}
