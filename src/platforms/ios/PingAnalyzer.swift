/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

class PingAnalyzer {
    private let logger = IOSLoggerImpl(tag: "PingAnalyzer")

    private let numberOfPings = 20
    private let pingSendTimespan: Double = 5.0 // send numberOfPings over 5 secons

    private var checkTime: Double {
        return pingSendTimespan + pingTimeNoSignalSeconds
    }
    private var timer: Timer = Timer()

    // In seconds, how often to send a ping
    private var pingSendTimeSeconds: Double {
        return pingSendTimespan / Double(numberOfPings)
    }

    private var pingDurations: [TimeInterval] = []

    private let pingTimeUnstableSeconds: Double = 1
    private let pingTimeNoSignalSeconds: Double = 4.0
    private let pingLossUnstableThreshold: Double = 0.10
    private let pingLossNoSignalThreshold: Double = 0.20

    private let callback: (ConnectionHealth.ConnectionStability?) -> Void

    init(pingAddress: String, callback: @escaping (ConnectionHealth.ConnectionStability?) -> Void) {
        self.callback = callback

        do {
            let ping = try SwiftyPing(host: pingAddress, configuration: PingConfiguration(interval: pingSendTimeSeconds, with: pingTimeNoSignalSeconds), queue: DispatchQueue.global())
            ping.observer = { (response) in
                if let error = response.error {
                    self.logger.error(message: "Ping error: \(error)")
                    self.logger.error(message: "\(response)")
                    return
                }

                self.pingDurations.append(response.duration)

                if self.pingDurations.count == self.numberOfPings {
                    self.timer.invalidate()
                    self.calculateStability()
                }
            }
            ping.targetCount = numberOfPings
            logger.info(message: "Sending pings")
            try ping.startPinging()
        } catch {
            logger.error(message: "Error when sending pings")
            callback(nil)
        }
        timer = Timer.scheduledTimer(timeInterval: TimeInterval(checkTime), target: self, selector: #selector(calculateStability), userInfo: nil, repeats: false)
    }

    // This will be called when all pings return or the timer completes, whichever comes first.
    @objc func calculateStability() {
        let packetLoss = numberOfPings - pingDurations.count
        let packetLossPercent = Double(packetLoss) / Double(numberOfPings)
        let longestPingTime = pingDurations.max() ?? 100 // in the rare situation all pings are lost, max is nil and a default large value is used

        logger.error(message: "Calculated stability: Packet loss \(packetLoss), longest ping time \(longestPingTime)")

        // If any pings take too long to return or
        if (packetLossPercent > pingLossNoSignalThreshold) || (longestPingTime > pingTimeNoSignalSeconds) {
            callback(.noSignal)
        } else if (packetLossPercent > pingLossUnstableThreshold) || (longestPingTime > pingTimeUnstableSeconds) {
            callback(.unstable)
        } else {
            callback(.stable)
        }
    }
}
