# Mozilla VPN Daemon

On most platforms, the VPN software requires privileged access to the operating system in order to create and manage
network interfaces. This level of privilege is unsuitable for a GUI program, so the duties of the VPN software are
split into a graphical client, and a privileged daemon program. This document describes the behaviour of the daemon
and how it interacts with the system.

## Communication Protocol

Every platform has unique service management and IPC mechanisms which dictate how to start the daemon and communicate
with it, but the format of the message sent and received by the daemon follow a common format and describe a common
set of commands:
 - `activate`: Method called to create a VPN connection to a specific Wireguard peer. This method takes a JSON
   configuration object describing the connection to establish, and returns a boolean value indicating if the
   connection has been started. If the returned value is `true` then the connection will be completed asynchronously
   by the emission of either a `connected` or `disconnected` signal.

 - `deactivate`: Method called to stop all VPN connections to all Wireguard peers.

 - `status`: Method called to fetch the current state of the VPN. Returns a JSON object describing the primary
   VPN connection (eg: the one that routes to the internet).

 - `getLogs`: Method called to fetch log messages from the daemon. This is typically joined to the logs from the
   GUI client when exporting logs for debug and analysis purposes. This returns the contents of the logs concatenated
   together into a multiline string.

 - `cleanupLogs`: Method called to delete logs messages from the daemon.

 - `connected`: An asynchronous signal sent from the daemon to inform the GUI client that a VPN connection to a
   Wireguard peer has been successfully established. The public key of the Wireguard peer is provided as an argument
   to this signal.

 - `disconnected`: An asychronous signal sent from the daemon to inform the GUI client that the VPN has been terminated
   and the connection to all Wireguard peers has been stoped.

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

Communication with the Linux daemon is achieved using the D-Bus protocol, described by the freedesktop.org
[D-Bus](http://dbus.freedesktop.org/doc/dbus-specification.html) specification. The daemon will be available on the
system bus, and takes ownership of the name `org.mozilla.vpn.dbus`.

The Linux kernel includes support for the Wireguard protocol, so this daemon only needs to respond to the communication
protocol and retain the necessary capabilities to orchestrate interface bringup, configuration and management.

### MacOS launchd

The MacOS daemon is launched using [launchd](https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPSystemStartup/Chapters/CreatingLaunchdJobs.html)
by installing a daemon plist at `/Library/LaunchDaemons/org.mozilla.macos.FirefoxVPN.daemon.plist` and registering the
daemon for automatic startup with `launchctl load -w $DAEMON_PLIST_PATH`.

Communication with the daemon is achieved via a named UNIX socket, which the daemon will create at `/var/run/mozillavpn/daemon.socket`.
The API methods are invoked by formatting the arguments as a JSON object, and inserting a value of `type` set to the name
of the method being invoked.

MacOS does not include native support for the Wireguard protocol, so in order to establish a Wireguard tunnel the daemon
will create and manage other processes to perform this function. The [wireguard-go](https://git.zx2c4.com/wireguard-go)
project provides a packet tunnel for Wireguard connectivity and connects to the kernel using the `/dev/net/tun` interface.

To manage the DNS resolver changes required for this interface, we also include another tool, the `macosdnsmanager` whose
purpose is to reconfigure MacOS to use the DNS resolver provided by the VPN network, and to restore the previous DNS 
configuration upon teardown of the VPN tunnel.

### Windows Service

The MSI Installer package creates a Windows service, named the `Mozilla VPN (broker)` to retain administrator privileges
in order to create and manage network interfaces and configuration. This service is set to launch automatically on boot,
and can be inspected via the Windows Service Manager.

Communication with the daemon is achieved via a named pipe, which the daemon will create at `\\.\pipe\mozillavpn`.
The API methods are invoked by formatting the arguments as a JSON object, and inserting a value of `type` set to the name
of the method being invoked.

Windows does not include native support for the Wireguard protocol, so in order provide this capability the
[Wireguard NT](https://github.com/WireGuard/wireguard-nt) kernel driver is installed by the windows service. This driver is
instatiated when bringing up the VPN tunnel to handle the connection.

Additionally, a [split-tunneling](https://github.com/mullvad/win-split-tunnel) driver may also be provided, which can be used
by the VPN to allow some applications to access the network directly and bypass the VPN. The daemon will check for this driver
at startup, and register it for split tunneling if possible.

### Android daemon???

TODO: How does Android work, exactly?

### iOS Network Extension

In order to provide custom VPN protocols on iOS, it is required to implement an iOS Network Extension. The Mozilla VPN
makes use of the [wireguard-apple](https://github.com/WireGuard/wireguard-apple) project to provide a network extension
implementing the [NEPacketTunnelProvider](https://developer.apple.com/documentation/networkextension/nepackettunnelprovider)
class.

## Crashes and Recovery
