/* Minimal extension-local helper to access the existing NETunnelProviderManager and session.
 * Place this file in the Widget/Intent extension target. */

import Foundation
import NetworkExtension

enum ExtensionTunnel {
    struct Context {
        let manager: NETunnelProviderManager
        let session: NETunnelProviderSession
    }

    static func loadContext(bundleId: String) async throws -> Context? {
        try await withCheckedThrowingContinuation { continuation in
            NETunnelProviderManager.loadAllFromPreferences { managers, error in
                if let error = error {
                    continuation.resume(throwing: error)
                    return
                }
                guard
                    let managers = managers,
                    let manager = managers.first(where: { mgr in
                        guard
                            let proto = mgr.protocolConfiguration as? NETunnelProviderProtocol,
                            let id = proto.providerBundleIdentifier
                        else { return false }
                        return id == bundleId
                    }),
                    let session = manager.connection as? NETunnelProviderSession
                else {
                    continuation.resume(returning: nil)
                    return
                }
                continuation.resume(returning: Context(manager: manager, session: session))
            }
        }
    }
}
