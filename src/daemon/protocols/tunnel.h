/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QDateTime>
#include <QJsonObject>

#include "interfaceconfig.h"
#include "models/server.h"

/**
 * The Tunnel class provides a unified interface for managing VPN connections
 * regardless of the underlying protocol (WireGuard, MASQUE, etc.). It handles
 * connection lifecycle, server switching, and maintains connection state.
 *
 * Key responsibilities:
 * - Managing tunnel interface creation and deletion
 * - Handling server connections and switches
 * - Tracking connection state and handshake completion
 * - Supporting split tunneling
 *
 * This is an abstract class. Platform and protocol-specific implementations
 * must override the pure virtual methods.
 */

class Tunnel : public QObject {
  Q_OBJECT
 public:
  // Operation types for tunnel management
  enum Op {
    Up,      ///< Bring tunnel up and activate connection
    Down,    ///< Tear down tunnel and deactivate connection
    Switch,  ///< Switch to a different server without full restart
  };
  explicit Tunnel(QObject* parent) : QObject(parent){};
  virtual ~Tunnel() = default;

  /**
   * @brief Activates the tunnel with the specified configuration
   * @param config Interface configuration containing server details, secrets,
   * and routing
   * @return true if activation succeeded, false otherwise
   *
   * This method brings up the tunnel interface, establishes the connection,
   * and configures routing according to the provided configuration.
   */
  virtual bool activate(const InterfaceConfig& config) = 0;

  /**
   * @brief Switches to a different server without tearing down the tunnel
   * @param config New interface configuration for the target server
   * @return true if server switch succeeded, false otherwise
   *
   * This is an optimization to change servers without full tunnel teardown.
   * Only possible when certain configuration parameters remain unchanged
   * (see supportServerSwitching()).
   */
  virtual bool switchServer(const InterfaceConfig& config) = 0;

  /**
   * @brief Retrieves current tunnel status as a JSON object
   * @return JSON object containing connection status, bandwidth stats, and
   * metadata
   *
   * Typical fields include:
   * - connected: boolean indicating connection state
   * - serverIpv4Gateway: IPv4 gateway address
   * - deviceIpv4Address: Device's assigned IPv4
   * - date: Connection establishment timestamp
   * - txBytes: Transmitted bytes
   * - rxBytes: Received bytes
   */
  virtual QJsonObject getStatus() const = 0;

  /**
   * @brief Checks for completed handshakes on pending connections
   * @return Number of connections still awaiting handshake completion
   *
   * For protocols with handshake mechanisms (e.g., WireGuard), this checks
   * if pending connections have completed their handshake. Returns 0 when
   * all connections are established.
   * Maybe rename to checkPendingConnections
   */
  virtual int checkHandshake() = 0;

  /**
   * @brief Returns the protocol type of this tunnel implementation
   * @return Protocol type (WireGuard, MASQUE, etc.)
   */
  virtual Server::ProtocolType protocolType() const = 0;

  /**
   * @brief Checks if the tunnel interface currently exists on the system
   * @return true if interface exists, false otherwise
   */
  virtual bool interfaceExists() = 0;

  /**
   * @brief Returns the name of the tunnel interface
   * @return Interface name
   */
  virtual QString interfaceName() const = 0;

  /**
   * @brief Creates the tunnel interface with the specified configuration
   * @param config Interface configuration containing protocol specific params
   * @return true if interface creation succeeded, false otherwise
   *
   * This creates the underlying network interface but doesn't necessarily
   * establish the connection yet (see activate()).
   */
  virtual bool addInterface(const InterfaceConfig& config) = 0;

  /**
   * @brief Deletes the tunnel interface
   * @return true if interface deletion succeeded, false otherwise
   *
   * This removes the interface from the system and cleans up associated
   * resources (routes, firewall rules, etc.).
   * This method should be called by Daemon::deactivate
   */
  virtual bool deleteInterface() = 0;

  // Methods for split routings

  /**
   * @brief Excludes a specific application from routing through the VPN tunnel
   * @param app Application identifier
   *
   * Implementation depends on platform capabilities.
   */
  virtual void excludeApp(const QString& app) = 0;

  /**
   * @brief Resets routing for an application back to default (through VPN)
   * @param app Application identifier to reset
   *
   * Removes any special routing rules for the application, causing its traffic
   * to go through the VPN tunnel again.
   */
  virtual void resetApp(const QString& app) = 0;

  /**
   * @brief Resets routing for all applications to default (through VPN)
   */
  virtual void resetAllApps() = 0;

  /**
   * @brief Indicates it this tunnel implementation supports split tunneling
   * @return true if split tunneling is supported, false otherwise
   *
   */
  virtual bool supportSplitTunnel() = 0;

  /**
   * @brief Checks if seamless server switching is supported for the given
   * config
   * @param config Target configuration to switch to
   * @return true if server can be switched without full reconnection, false
   * otherwise
   *
   * When not supported, a full disconnect/reconnect cycle is required.
   */
  virtual bool supportServerSwitching(const InterfaceConfig& config);

  /**
   * @brief Executes a tunnel operation (Up, Down, or Switch)
   * @param op Operation to perform
   * @param config Configuration for the operation
   * @return true if operation succeeded, false otherwise
   *
   * This is the main state transition method. It updates internal connection
   * state and delegates to activate() for Up operations.
   */
  virtual bool run(Op op, const InterfaceConfig& config);

  /**
   * @brief Removes the tunnel interface if it exists
   *
   * This is a safety method called during Daemon initialization to ensure
   * no stale interfaces exist from previous runs. Calls qFatal() if the
   * interface exists but cannot be deleted.
   */
  void removeInterfaceIfExists();

  /**
   * @class ConnectionState
   * @brief Represents the state of an active tunnel connection
   *
   * Tracks connection metadata including establishment time and configuration.
   * Used for both single-hop and multi-hop connections.
   */
  class ConnectionState {
   public:
    ConnectionState(){};
    ConnectionState(const InterfaceConfig& config) { m_config = config; }
    QDateTime m_date;
    InterfaceConfig m_config;
  };
  QMap<InterfaceConfig::HopType, ConnectionState> connections() const {
    return m_connections;
  }

 signals:
  void backendFailure();
  void connected(const QString& pubkey);

 protected:
  /**
   * @brief Map of active connections indexed by hop type
   */
  QMap<InterfaceConfig::HopType, ConnectionState> m_connections;
};

#endif  // PROTOCOL_H