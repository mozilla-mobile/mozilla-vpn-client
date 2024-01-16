/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

class TunnelError: Error {

}

// Singleton that manages the VPN tunnel and exposes functions
// for interacting with it.
class TunnelManager {
    private static let logger = IOSLoggerImpl(tag: "TunnelManager")
    
    static var vpnBundleId = ""

    private static let instance = TunnelManager()

    private var tunnel: NETunnelProviderManager?
    
    static var session: NETunnelProviderSession? {
            return TunnelManager.instance.tunnel?.connection as? NETunnelProviderSession
    }
    
    static var protocolConfiguration: NETunnelProviderProtocol? {
        return TunnelManager.instance.tunnel?.protocolConfiguration as? NETunnelProviderProtocol
    }

    private init() {}
    
    static func initialize(_ bundleId: String, _ completionHandler: @escaping (_ error: Error?, _ tunnel: NETunnelProviderManager?) -> Void) {
        precondition(!bundleId.isEmpty)
        vpnBundleId = bundleId;
        
        logger.debug(message: "Attempting to initialize VPN tunnel")
        
        TunnelManager.instance.setTunnel {
            guard let tunnel = TunnelManager.instance.tunnel else {
                completionHandler(TunnelError(), nil)
                return
            }

            logger.info(message: "Tunnel initialized succesfully")
            completionHandler(nil, tunnel)
        }
    }
    
    @discardableResult static func withTunnel(_ f: (_ tunnel: NETunnelProviderManager) throws -> Any) -> Any? {
        guard let tunnel = TunnelManager.instance.tunnel else {
            logger.error(message: "Attempted to use the VPN tunnel, but it's not available.")
            return nil
        }

        do {
            let result = try f(tunnel)
            return result
        } catch {
            logger.error(message: "Error executing callback: \(error).")
            return nil
        }
    }

    private func setTunnel(_ completionHandler: @escaping () -> Void) {
        NETunnelProviderManager.loadAllFromPreferences { [weak self] managers, error in
            guard let self = self else {
                TunnelManager.logger.debug(message: "We are shutting down.")
                completionHandler();
                return
            }

            if let error = error {
                TunnelManager.logger.error(message: "Loading from preference failed: \(error)")
                completionHandler();
                return
            }

            let nsManagers = managers ?? []
            TunnelManager.logger.debug(message: "We have received \(nsManagers.count) managers.")

            guard let tunnel = nsManagers.first(where: TunnelManager.isOurManager(_:)) else {
                TunnelManager.logger.debug(message: "Creating the tunnel")
                self.tunnel = NETunnelProviderManager()
                completionHandler();
                return
            }

            TunnelManager.logger.debug(message: "Tunnel already exists")
            self.tunnel = tunnel
            completionHandler()
        }
    }

    private static func isOurManager(_ manager: NETunnelProviderManager) -> Bool {
        guard
            let proto = manager.protocolConfiguration,
            let tunnelProto = proto as? NETunnelProviderProtocol
        else {
            logger.debug(message: "Ignoring manager because the proto is invalid.")
            return false
        }

        guard let bundleIdentifier = tunnelProto.providerBundleIdentifier else {
            logger.debug(message: "Ignoring manager because the bundle identifier is null.")
            return false
        }

        if (bundleIdentifier != vpnBundleId) {
            logger.debug(message: "Ignoring manager because the bundle identifier doesn't match.")
            return false;
        }

        logger.debug(message: "Found the manager with the correct bundle identifier: \(bundleIdentifier)")
        return true
    }
}
