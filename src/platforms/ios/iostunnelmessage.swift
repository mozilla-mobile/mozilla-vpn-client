/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import NetworkExtension

/// Enum listing the possible messages the main app 
/// can send to the VPN tunnel process
public enum TunnelMessage: Codable, CustomStringConvertible {
    /// Request for the current runtime configuration
    case getRuntimeConfiguration

    /// Request for the original connection timestamp
    case getConnectionTimestamp

    /// Switch tunnel configuration
    case configurationSwitch(String)

    /// Telemetry state has changed
    case telemetryEnabledChanged(Bool)

    /// Manually initiate a network extension silent server switch (used for testing only)
    case silentServerSwitch

    public var description: String {
        switch self {
        case .getRuntimeConfiguration:
            return "getRuntimeConfiguration"
        case .getConnectionTimestamp:
            return "getConnectionTimestamp"
        case .configurationSwitch(let newConfig):
            // We do not want to log the configuration itself.
            // It contains private information.
            return "configurationSwitch"
        case .telemetryEnabledChanged(let flag): 
            return "telemetryEnabledChanged(\(flag))"
        case .silentServerSwitch:
            return "silentServerSwitch"
        }
    }

    public init(_ messageData: Data) throws {
        self = try JSONDecoder().decode(Self.self, from: messageData)
    }

    public func encode() throws -> Data {
        return try JSONEncoder().encode(self)
    }
}
