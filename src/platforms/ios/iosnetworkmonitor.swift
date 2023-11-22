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

    public var onNewConnectedPath: ((Bool) -> Void)?

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
            
            self.onNewConnectedPath?(areAllGatewaysPrivate)
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
    
    // Function to check if an IPv4 address is in RFC 1918 range
    private static func isRFC1918IPv4(address: Data) -> Bool {
        return (
            (address[0] == 10) ||
            (address[0] == 172 && address[1] >= 16 && address[1] <= 31) ||
            (address[0] == 192 && address[1] == 168)
        )
    }

    // Function to check if an IPv6 address is in RFC 4193 range
    private static func isRFC4193IPv6(address: Data) -> Bool {
        return address.starts(with: [0xfd])
    }

    private static func isPrivateGateway(gateway: NWEndpoint) -> Bool {
        switch gateway {
            case let .hostPort(host: host, port: _):
                switch host {
                case let .name(hostname, _):
                    // Convert the hostname to an array of integers for IPv4
                    let components = hostname.split(separator: ".").compactMap { Int($0) }
                    
                    if components.count == 4 {
                        return IOSNetworkMonitor.isRFC1918IPv4(address: Data(components.map { UInt8($0) }))
                    } else if components.count == 8 && hostname.lowercased().hasPrefix("fc") && !hostname.contains("::") {
                        return true
                    } else {
                        return false
                    }
                case let .ipv4(address):
                    return IOSNetworkMonitor.isRFC1918IPv4(address: address.rawValue)
                case let .ipv6(address):
                    return IOSNetworkMonitor.isRFC4193IPv6(address: address.rawValue)
                default:
                    IOSNetworkMonitor.logger.error(message: "IMPOSSIBLE: Unexpected host port type. Assuming unsafe.")
                    return false
                }
            default:
                IOSNetworkMonitor.logger.error(message: "IMPOSSIBLE: Unexpected endpoint type. Assuming unsafe.")
                return false
            }
    }
}
