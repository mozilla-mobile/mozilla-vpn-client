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

    private var onNewConnectedPath: ((Bool) -> Void)?

    init() {
        monitor = NWPathMonitor()
        monitor.pathUpdateHandler = { path in
            if (path.status != .satisfied) {
                IOSNetworkMonitor.logger.info(message: "New path update for unconnected path. Ignoring.")
                return
            }

            guard let currentlyConnectedInterface = path.availableInterfaces.first(where: { path.usesInterfaceType($0.type) }) else {
                IOSNetworkMonitor.logger.info(
                    message: "UNEXPECTED: Path is satisfied, but not using any interface. Currently available interfaces are: \(path.availableInterfaces.map { "\($0.type.toString())" })"
                )
                return
            }

            IOSNetworkMonitor.logger.info(message: "New connected path")
            IOSNetworkMonitor.logger.info(message: "Gateways: \(path.gateways)")
            IOSNetworkMonitor.logger.info(message: "Currently connected interface: \(currentlyConnectedInterface.type.toString())")

            let areAllGatewaysPrivate = path.gateways.reduce(true) { result, gateway in
                return result && IOSNetworkMonitor.isPrivateGateway(gateway: gateway)
            }

            IOSNetworkMonitor.logger.info(message: "Are all gateways private? \(areAllGatewaysPrivate ? "Yes." : "No.")")

            onNewConnectedPath?(areAllGatewaysPrivate)
        }
    }

    deinit {
        stop();
    }

    func start(onNewConnectedPath: ((Bool) -> Void)) {
        IOSNetworkMonitor.logger.info(message: "Starting network monitoring")
        monitor.start(queue: queue)
    }

    func stop() {
        IOSNetworkMonitor.logger.info(message: "Stopping network monitoring")
        monitor.cancel()
    }

    private static func isPrivateGateway(gateway: NWEndpoint) {
        switch gateway {
        case let .hostPort(host: host, port: _):
            switch host {
            case let .name(hostname):
                // Convert the hostname to an array of integers for IPv4
                let components = hostname.split(separator: ".").compactMap { Int($0) }

                // Check for RFC 1918 private IP address ranges (for IPv4)
                let isRFC1918IPv4 = (
                    components.count == 4 &&
                    (components[0] == 10 ||
                    (components[0] == 172 && components[1] >= 16 && components[1] <= 31) ||
                    (components[0] == 192 && components[1] == 168))
                )

                // Check for RFC 4193 Unique Local IPv6 Unicast Addresses (for IPv6)
                let isRFC4193IPv6 = (
                    components.count == 8 &&
                    hostname.lowercased().hasPrefix("fc") &&
                    !hostname.contains("::")
                )

                // Return true if the host is in RFC 1918 or RFC 4193 ranges
                return isRFC1918IPv4 || isRFC4193IPv6

            case let .ipv4(address):
                // Check for RFC 1918 private IP address ranges (for IPv4)
                let isRFC1918IPv4 = (
                    (address[0] == 10) ||
                    (address[0] == 172 && address[1] >= 16 && address[1] <= 31) ||
                    (address[0] == 192 && address[1] == 168)
                )

                return isRFC1918IPv4

            case let .ipv6(address):
                // Check for RFC 4193 Unique Local IPv6 Unicast Addresses (for IPv6)
                let isRFC4193IPv6 = (
                    address.starts(with: [0xfd])
                )

                return isRFC4193IPv6
            }
        default:
            return false
        }
    }
}
