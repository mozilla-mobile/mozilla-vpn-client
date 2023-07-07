/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import Foundation
import NetworkExtension

// Singleton that manages the VPN tunnel and exposes functions
// for interacting with it.
class TunnelManager {
    private static let logger = IOSLoggerImpl(tag: "TunnelManager")
    
    static var vpnBundleId = ""

    private static let instance = TunnelManager()

    private var tunnel: Result<NETunnelProviderManager, Error>?
    
    static var session: NETunnelProviderSession? {
        get {
            return withTunnel { tunnel in
                return (tunnel.connection as? NETunnelProviderSession)!
            } as! NETunnelProviderSession?
        }
    }
    
    static var protocolConfiguration: NETunnelProviderProtocol? {
        get {
            return withTunnel { tunnel in
                return (tunnel.protocolConfiguration as? NETunnelProviderProtocol)!
            } as! NETunnelProviderProtocol?
        }
    }

    private init() {}
    
    static func initialize(_ bundleId: String, _ completionHandler: @escaping (_ error: Error?, _ tunnel: NETunnelProviderManager?) -> Void) {
        precondition(!bundleId.isEmpty)
        vpnBundleId = bundleId;
        
        logger.debug(message: "Attempting to initialize VPN tunnel")
        
        TunnelManager.instance.setTunnel { 
            switch TunnelManager.instance.tunnel {
            case .failure(let error):
                logger.error(message: "Error initializing tunnel: \(error).")
                return completionHandler(error, nil)
            case .success(let tunnel):
                logger.info(message: "Tunnel initialized succesfully")
                return completionHandler(nil, tunnel)
            case .none:
                fatalError("IMPOSSIBLE: Attempted to set the VPN tunnel, but didn't get an error nor a tunnel.")
            }
            
        };
    }
    
    static func withTunnel(_ f: (_ tunnel: NETunnelProviderManager) throws -> Any) -> Any? {
        switch TunnelManager.instance.tunnel {
        case .success(let tunnel):
            do {
                let result = try f(tunnel)
                return result
            } catch {
                logger.error(message: "Error executing callback: \(error).")
                return nil
            }
        case .failure, .none:
            logger.error(message: "Attempted to use the VPN tunnel, but it's not available.")
            return nil
        }
    }

    private func setTunnel(_ completionHandler: @escaping () -> Void) {
        NETunnelProviderManager.loadAllFromPreferences { [weak self] managers, error in
            if self == nil {
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

            let tunnel = nsManagers.first(where: TunnelManager.isOurManager(_:))
            if tunnel == nil {
                TunnelManager.logger.debug(message: "Creating the tunnel")
                self!.tunnel = .success(NETunnelProviderManager())
                completionHandler();
                return
            }

            TunnelManager.logger.debug(message: "Tunnel already exists")
            self!.tunnel = .success(tunnel!)
            completionHandler();
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

        if (tunnelProto.providerBundleIdentifier == nil) {
            logger.debug(message: "Ignoring manager because the bundle identifier is null.")
            return false
        }

        if (tunnelProto.providerBundleIdentifier != vpnBundleId) {
            logger.debug(message: "Ignoring manager because the bundle identifier doesn't match.")
            return false;
        }

        logger.debug(message: "Found the manager with the correct bundle identifier: \(tunnelProto.providerBundleIdentifier!)")
        return true
    }
}
