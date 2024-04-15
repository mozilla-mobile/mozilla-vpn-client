/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension
import IOSGlean

protocol SilentServerSwitching: AnyObject {
    func silentServerSwitch()
}

class ConnectionHealth {

    // Every 15-16 seconds, send 20 pings over 5 seconds.
    // Pings timeout after 4 seconds.
    // After all pings have returned or timed out,
    // PingAnalyzer calculates the stability.

    enum ConnectionStability {
        case stable, unstable, noSignal
    }

    private let logger = IOSLoggerImpl(tag: "ConnectionHealthSwift")

    weak var serverSwitchingDelegate: SilentServerSwitching?

    // Timer should fire between 15 and 16 seconds.
    private let checkTime = 15.0 // 15 seconds
    private let toleranceTime = 1.0 // 1 seconds
    private var timer: Timer?

    private var lastHealthStatus: ConnectionStability = .stable
    private var connectionHealthTimerId: GleanTimerId? = nil

    private var pingAddress: String?

    func start(for pingAddress: String) {
        self.logger.info(message: "Starting ConnectionHealth with \(pingAddress)")
        self.pingAddress = pingAddress
        startTimer()
        startTimingDistributionMetric(for: lastHealthStatus)
    }

    func startTimer() {
        // Need to put timer on the main thread to get a runloop: https://stackoverflow.com/questions/38164120/why-would-a-scheduledtimer-fire-properly-when-setup-outside-a-block-but-not-w
        DispatchQueue.main.sync {
            // There is no restart on a Timer (though we repeat it), so we need to recreate it each time the VPN reconnects
            timer = Timer.scheduledTimer(timeInterval: checkTime, target: self, selector: #selector(runHealthChecks), userInfo: nil, repeats: true)
        }
        // Giving Timer some tolerance improves performance from iOS.
        timer?.tolerance = toleranceTime
    }

    func stop() {
        self.logger.info(message: "Stopping ConnectionHealth timer")
        stopTimingDistributionMetric(for: lastHealthStatus)
        guard let confirmedTimer = timer else {
            logger.error(message: "No timer found when stopping ConnectionHealth")
            return
        }
        confirmedTimer.invalidate()
        timer = nil
    }

    @objc func runHealthChecks() {
        guard let pingAddress = pingAddress else {
            logger.error(message: "No ping address when starting health checks")
            return
        }

        logger.info(message: "Creating PingAnalyzer")
        let _ = PingAnalyzer(pingAddress: pingAddress) { (connectivity) in
            guard let connectivity = connectivity else {
                self.logger.error(message: "PingAnalyzer returned error")
                return
            }

            self.logger.info(message: "ConnectionHealth connectivity: \(connectivity)")
            self.recordMetrics(with: connectivity)

            if connectivity == .unstable {
                self.logger.info(message: "Unstable, starting silent switch from network extension")
                self.serverSwitchingDelegate?.silentServerSwitch()
            }
        }

        // Timer set to repeat until stop() is run, so no need to call any repeat step here.
    }

    private func recordMetrics(with stability: ConnectionStability) {
        switch stability {
        case .stable: GleanMetrics.ConnectionHealth.stableCount.add()
        case .unstable: GleanMetrics.ConnectionHealth.unstableCount.add()
        case .noSignal: GleanMetrics.ConnectionHealth.noSignalCount.add()
        }

        if (lastHealthStatus == stability) {
            return
        }

        logger.info(message: "Health status changed.")

        switch stability {
        case .stable: GleanMetrics.ConnectionHealth.changedToStable.record()
        case .unstable: GleanMetrics.ConnectionHealth.changedToUnstable.record()
        case .noSignal: GleanMetrics.ConnectionHealth.changedToNoSignal.record()
        }

        stopTimingDistributionMetric(for: lastHealthStatus)
        startTimingDistributionMetric(for: stability)

        lastHealthStatus = stability
    }

    private func startTimingDistributionMetric(for stability: ConnectionStability) {
        switch stability {
        case .stable: connectionHealthTimerId = GleanMetrics.ConnectionHealth.stableTime.start()
        case .unstable: connectionHealthTimerId = GleanMetrics.ConnectionHealth.unstableTime.start()
        case .noSignal: connectionHealthTimerId = GleanMetrics.ConnectionHealth.noSignalTime.start()
        }
    }

    private func stopTimingDistributionMetric(for stability: ConnectionStability) {
        guard let connectionHealthTimerId = connectionHealthTimerId else {
            logger.error(message: "No active health timer for \(stability)")
            return
        }

        switch stability {
        case .stable: GleanMetrics.ConnectionHealth.stableTime.stopAndAccumulate(connectionHealthTimerId)
        case .unstable: GleanMetrics.ConnectionHealth.unstableTime.stopAndAccumulate(connectionHealthTimerId)
        case .noSignal: GleanMetrics.ConnectionHealth.noSignalTime.stopAndAccumulate(connectionHealthTimerId)
        }

        // Set to nil to defensively ensure there is no future erroenous attempt to turn it off
        self.connectionHealthTimerId = nil

    }
}
