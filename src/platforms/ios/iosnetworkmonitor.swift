/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import Network

final class NetworkMonitor {
    private static let logger = IOSLoggerImpl(tag: "NetworkMonitor")

    private let queue = DispatchQueue(label: "NetworkMonitorQueue")
    private let monitor: NWPathMonitor

    private init() {
        monitor = NWPathMonitor()
        monitor.pathUpdateHandler = { [weak self] path in
            if (path.status == .satisfied) {

            }
        }
    }

    func start() {
        monitor.start(queue: queue)
    }

    func stop() {
        monitor.cancel()
    }
}
