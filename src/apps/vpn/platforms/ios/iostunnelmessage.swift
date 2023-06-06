/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import NetworkExtension

/// Enum listing the possible messages the main app 
/// can send to the VPN tunnel process
public enum TunnelMessage: Codable, CustomStringConvertible {
    /// Request for the current runtime configuration
    case getRuntimeConfiguration

    /// Switch tunnel configuration
    case configurationSwitch(String)

    /// Telemetry state has changed
    case telemetryEnabledChanged(Bool)

    public var description: String {
        switch self {
        case .getRuntimeConfiguration:
            return "getRuntimeConfiguration"
        case .configurationSwitch(let newConfig):
            return "configurationSwitch(\(newConfig))"
        case .telemetryEnabledChanged(let flag):
            return "telemetryEnabledChanged(\(flag))"
        }
    }

    public init(_ messageData: Data) throws {
        self = try JSONDecoder().decode(Self.self, from: messageData)
    }

    public func encode() throws -> Data {
        return try JSONEncoder().encode(self)
    }

    public func send(handleResult: ((Data?) -> Void)? = nil) {
        Logger.global?.log(message: "Sending message to VPN tunnel: \(self).")
        
        let _ = TunnelManager.withTunnel { tunnel in
            guard let session = tunnel.connection as? NETunnelProviderSession
            else {
                handleResult?(nil)
                return
            }
            
            let messageData: Data
            do {
                messageData = try self.encode()
            } catch {
                Logger.global?.log(message: "Error encoding message: \(self). Message not sent.")
                handleResult?(nil)
                return
            }

            do {
                try session.sendProviderMessage(messageData) { [handleResult] data in
                    handleResult?(data)
                }

                Logger.global?.log(message: "Message '\(self)' sent.")
                return
            } catch {
                Logger.global?.log(message: "Error sending message '\(self)': \(error)")
                handleResult?(nil)
                return
            }
        }
    }
}
