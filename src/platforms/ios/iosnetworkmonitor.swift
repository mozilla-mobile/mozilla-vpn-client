/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import Network

extension NWInterface.InterfaceType {
    func toString() -> String {
        switch self {
        case .wifi:
            return "Wi-Fi"
        case .cellular:
            return "Cellular"
        case .wiredEthernet:
            return "Wired Ethernet"
        default:
            // Other or loopback,
            // doesn't really matter.
            return "Unknown"

        }
    }
}

final class IOSNetworkMonitor {
    private static let logger = IOSLoggerImpl(tag: "NetworkMonitor")

    private let queue = DispatchQueue(label: "NetworkMonitorQueue")
    private let monitor: NWPathMonitor

    init() {
        monitor = NWPathMonitor()
        monitor.pathUpdateHandler = { path in
            if (path.status == .satisfied) {
                IOSNetworkMonitor.logger.info(message: "New connected path")
                IOSNetworkMonitor.logger.info(message: "Gateways: \(path.gateways)")
                IOSNetworkMonitor.logger.info(message: "Available interfaces types: \(path.availableInterfaces.map { "\($0.type.toString()): \(path.usesInterfaceType($0.type))" })")
            }
        }
    }

    deinit {
        stop();
    }

    func start() {
        IOSNetworkMonitor.logger.info(message: "Starting network monitoring")
        monitor.start(queue: queue)
    }

    func stop() {
        IOSNetworkMonitor.logger.info(message: "Stopping network monitoring")
        monitor.cancel()
    }
}
