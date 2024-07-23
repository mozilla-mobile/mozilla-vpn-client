# Mozilla VPN Daemon

On most platforms, the VPN software requires privileged access to the operating system in order to create and manage
network interfaces. This level of privilege is unsuitable for a GUI program, so the duties of the VPN software are
split into a graphical client, and a privileged daemon program. This document describes the behavior of the daemon
and how it interacts with the system.

## Communication Protocol

Every platform has unique service management and interprocess communication mechanisms which dictate how to start the
daemon and communicate with it, but the format of the message sent and received by the daemon follow a common format
and describe a common set of commands:
 - `activate`: Method called to create a VPN connection to a specific Wireguard peer. This method takes a JSON
   configuration object describing the connection to establish, and returns a boolean value indicating if the
   connection has been started. If the returned value is `true` then the connection will be completed asynchronously
   by the emission of either a `connected` or `disconnected` signal.

 - `deactivate`: Method called to stop all VPN connections to all Wireguard peers.

 - `status`: Method called to fetch the current state of the VPN. Returns a JSON object describing the primary
   VPN connection (the one that routes to the internet).

 - `getLogs`: Method called to fetch log messages from the daemon. This is typically joined to the logs from the
   GUI client when exporting logs for debug and analysis purposes. This returns the contents of the logs concatenated
   together into a multi-line string.

 - `cleanupLogs`: Method called to delete logs messages from the daemon.

 - `connected`: An asynchronous signal sent from the daemon to inform the GUI client that a VPN connection to a Wireguard
   peer has been successfully established. The public key of the Wireguard peer is provided as an argument to this
   signal.

 - `disconnected`: An asynchronous signal sent from the daemon to inform the GUI client that the VPN has been terminated
   and the connection to all Wireguard peers has been stopped.

## Platform Implementations

### Linux systemd service

The Linux daemon is launched as a systemd service named `mozillavpn`. You can manage this service using the `systemctl`
command. For example:

```
[user@hostname ~] systemctl status mozillavpn
o mozillavpn.service - MozillaVPN D-Bus service
     Loaded: loaded (/usr/lib/systemd/system/mozillavpn.service; enabled; preset: disabled)
    Drop-In: /usr/lib/systemd/system/service.d
               10-timeout-abort.conf
     Active: active (running) since Mon 2024-07-22 14:22:34 PDT; 1s ago
   Main PID: 40698 (mozillavpn)
      Tasks: 9 (limit: 17683)
     Memory: 21.9M
        CPU: 492ms
     CGroup: /system.slice/mozillavpn.service
               40698 /usr/bin/mozillavpn linuxdaemon 
```

Other commands include:
 - `systemctl enable mozillavpn`: Enable the daemon to start when the system boots.
 - `systemctl disable mozillavpn`: Disable startup of them daemon when the system boots.
 - `systemctl start mozillavpn`: Request startup of the daemon.
 - `systemctl stop mozillavpn`: Request shutdown of the daemon.
 - `systemctl restart mozillavpn`: Stop and restart the daemon.

Communication with the Linux daemon is achieved using the D-Bus protocol, described by the FreeDesktop
[D-Bus](http://dbus.freedesktop.org/doc/dbus-specification.html) specification. The daemon will be available on the
system bus, and takes ownership of the name `org.mozilla.vpn.dbus`.

The Linux kernel includes support for the Wireguard protocol, so this daemon only needs to respond to the communication
protocol and retain the necessary capabilities to orchestrate interface creation, configuration and management.

### MacOS launchd

The MacOS daemon is launched using [launchd](https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPSystemStartup/Chapters/CreatingLaunchdJobs.html)
by installing a daemon plist at `/Library/LaunchDaemons/org.mozilla.macos.FirefoxVPN.daemon.plist` and registering the
daemon for automatic startup with `launchctl load -w $DAEMON_PLIST_PATH`.

Communication with the daemon is achieved via a named UNIX socket, which the daemon will create at
`/var/run/mozillavpn/daemon.socket`. The API methods are invoked by formatting the arguments as a JSON object, and
inserting a value of `type` set to the name of the method being invoked.

MacOS does not include native support for the Wireguard protocol, so in order to establish a Wireguard tunnel, the
daemon will create and manage other processes to perform this function. The
[wireguard-go](https://git.zx2c4.com/wireguard-go) project provides a packet tunnel for Wireguard connectivity and
connects to the kernel using the `/dev/net/tun` interface.

To manage the DNS resolver changes required for this interface, we also include another tool, the `macosdnsmanager`,
whose purpose is to reconfigure MacOS to use the DNS resolver provided by the VPN network, and to restore the previous
DNS  configuration upon shutdown of the VPN tunnel.

### Windows Service

The MSI Installer package creates a Windows service, named the `Mozilla VPN (broker)`, to gain administrator privileges
for creating and managing the network interfaces and configuration. This service is set to launch automatically on boot,
and can be inspected via the Windows Service Manager.

Communication with the daemon is achieved via a named pipe, which the daemon will create at `\\.\pipe\mozillavpn`.
The API methods are invoked by formatting the arguments as a JSON object, and inserting a value of `type` set to the
name of the method being invoked.

Windows does not include native support for the Wireguard protocol, so in order provide this capability the
[Wireguard NT](https://github.com/WireGuard/wireguard-nt) kernel driver is installed by the windows service. This driver
is instantiated when bringing up the VPN tunnel to handle the connection.

Additionally, a [split-tunneling](https://github.com/mullvad/win-split-tunnel) driver may also be provided, which can be
used by the VPN to allow some applications to access the network directly and bypass the VPN. The daemon will check for
this driver at startup, and register it for split tunneling if possible.

### Android Service

The Android daemon is a [Service](https://developer.android.com/develop/background-work/services) configure to run in
the background, this ensures that the VPN tunnel can continue to run even if the GUI is suspended to save on resources.
Unlike the standard Android guidelines, this services is explicitly configured to run in a separate process.

Communication with the daemon is achieved using [bindings](https://developer.android.com/develop/background-work/services/bound-services). The protocol is similar to the desktop daemons, except that it takes the form of:
`{requestType:int, data:json}`

The VPN tunnel is run within the process by creating a raw TUN device via the
[VpnService.Builder](https://developer.android.com/reference/android/net/VpnService.Builder) API, and then creating a
[wireguard-go](https://git.zx2c4.com/wireguard-go) process to implement the Wireguard protocol.

### iOS Network Extension

In order to provide custom VPN protocols on iOS, it is required to implement an iOS Network Extension. The Mozilla VPN
makes use of the [wireguard-apple](https://github.com/WireGuard/wireguard-apple) project to provide a network extension
implementing the [NEPacketTunnelProvider](https://developer.apple.com/documentation/networkextension/nepackettunnelprovider)
class.

## Crashes and Recovery

Because the VPN is designed to intercept all network traffic originating from the user's device and encrypt it, a crash
or failure of this software can be especially severe, and has the potential to permanently break the user's internet
connectivity if handled incorrectly. Therefore additional steps are taken to ensure that a crash of this software can be
recovered from.

In the GUI client, software communication with the daemon is guarded by a timeout mechanism and communication errors
must be handled gracefully. These errors are detected by the `Controller` class, and reported as an
`ErrorHandler::ControllerError`. Such failures typically result in a re-initialization of the controller class which
will bring the client back to the deactivated state. Detection of these errors will trigger an error banner in the GUI
client, as well as a system tray notification in case the GUI client is not visible in the foreground.

Where possible the services are configured to automatically restart daemon processes which exit abnormally (crash):
 - Linux: The `mozillavpn.service` file sets `Restart=on-failure`
 - MacOS: The `org.mozilla.macos.FirefoxVPN.daemon.plist` set `KeepAlive` to `true`
 - Windows: The service config is set with `FirstFailureActionType` and `SecondFailureActionType` to `restart`

When starting, the daemon processes MUST check for the existence of any VPN interfaces, firewalls, kill-switches and
drivers which were created by a previous instance of the daemon and stop them as necessary to return to a known state
and to ensure that the host network connectivity is restored.

If the VPN connection was active at the time of the crash, the daemon SHOULD NOT attempt to re-activate the connection.
The context of such a connection is likely to have been lost in the crash and it is desired that the daemon should
remain stateless. An attempt to re-activate the connection may lead to the same condition which triggered the crash,
therefore we feel it is safer to leave the VPN inactive after recovery.

Because the daemon runs as a privileged process, has no UI, and is outside of direct user control, it does not currently
support crash reporting via Sentry. However, we can still get telemetry about the frequency of crashes and controller
communication errors through analysis of the `error_alert_shown` ping.
