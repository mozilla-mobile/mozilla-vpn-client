# Mozilla VPN - command-line interface

This quick document describes the command-line interface of the latest Mozilla
VPN client. The interface has not been stabilized yet and the commands, the
options and the output could change in the future.

The interface should be cross-platform but so far, I have been testing it only
on Linux.

## Help menu

```
% mozillavpn -h
usage: mozillavpn [-h | --help] [-v | --version] <commands> [<args>]

List of options:
  -h | --help          Displays help on commandline options.
  -v | --version       Displays version information.

List of commands:
  activate             Activate the VPN tunnel
  deactivate           Deactivate the VPN tunnel
  device               Remove a device by its id.
  login                Starts the authentication flow.
  logout               Logout the current user.
  select               Select a server.
  servers              Show the list of servers.
  status               Show the current VPN status.
  ui                   Start the UI.
  linuxdaemon          Starts the linux daemon
```

## UI interface

By default, without any parameters, the ‘ui’ command starts. This means that,
if I just execute ‘mozillavpn’, the main view will appear as expected. But
there are a couple of extra parameters I can use. See the help menu:

```
% mozillavpn ui -h
usage: mozillavpn ui [-h | --help] [-m | --minimized] [-s | --start-at-boot]

List of options:
  -h | --help          Displays help on commandline options.
  -m | --minimized     Start minimized.
  -s | --start-at-boot Start at boot (if configured).
```

## User status

The status request works in 2 modes: by cache only, or interacting with the
server. By default we do not use the cache. See the help menu:

```
% mozillavpn status -h
usage: mozillavpn status [-h | --help] [-c | --cache]

List of options:
  -h | --help          Displays help on commandline options.
  -c | --cache         From local cache.
```

The result with and without cache is the same. Cache is faster because no
network requests are done.

```
% mozillavpn status
User status: authenticated
User avatar: https://mozillausercontent.com/00000000000000000000000000000000
User displayName:
User email: baku@mozilla.com
User maxDevices: 5
User subscription needed: false
Active devices: 4
Current devices:prrrr ubuntu 20.04
Device 0 name: prrrr ubuntu 20.04
Device 0 creation time: Wed Nov 11 07:50:26 2020 GMT
Device 0 public key: l5E7frs36yLGrQSuAOe6vH6GYDmbvY4R2Br3MO1LfTU=
Device 0 ipv4 address: 10.68.12.139/32
Device 0 ipv6 address: fc00:bbbb:bbbb:bb01::5:c8a/128
Device 1 name: Andrea’s MacBook Pro
Device 1 creation time: Tue Nov 10 17:49:40 2020 GMT
Device 1 public key: rCPrXO+nnlTJt18lMM5b8H0gR16vZLpqXr4ygmopzUc=
Device 1 ipv4 address: 10.69.151.94/32
Device 1 ipv6 address: fc00:bbbb:bbbb:bb01::6:975d/128
Device 2 name: localhost android 8.0
Device 2 creation time: Fri Nov 6 14:52:19 2020 GMT
Device 2 public key: ajc4ySLPWoKvZBVt7043cC+0gCozqZE3rj55xsP85nk=
Device 2 ipv4 address: 10.68.89.192/32
Device 2 ipv6 address: fc00:bbbb:bbbb:bb01::5:59bf/128
Device 3 name: Andrea’s iPhone
Device 3 creation time: Tue Nov 3 11:28:16 2020 GMT
Device 3 public key: VAZ1nsfDPj502cGlGTuxp94t4KHwAn7TMlHJc3fzd0M=
Device 3 ipv4 address: 10.68.225.238/32
Device 3 ipv6 address: fc00:bbbb:bbbb:bb01::5:e1ed/128
Server country code: lu
Server country: Luxembourg
Server city: Luxembourg
VPN state: on
```

## Login

I can complete the authentication flow using ‘login’:

```
% mozillavpn login -h
unknown option: -h
usage: mozillavpn login
```

No output is shown during the login. The default browser is opened to show the
FxA authentication form. (this can be improved and allow the user to open the
URL as they like).

```
% mozillavpn login
```

## Logout

I can remove all the configurations, all my settings, logging out from the app.
No options are required:

```
% mozillavpn logout -h
unknown option: -h
usage: mozillavpn logout
```

No output is shown during the logout. Note that this removes the current device
registration on the server.

```
% mozillavpn logout
```

Any other operation will fail because the user is not authenticated:

```
% mozillavpn logout
User status: not authenticated

% mozillavpn status
User status: not authenticated
```

## Server list

I can retrieve the list of servers using the command ‘servers’:

```
% mozillavpn servers -h
usage: mozillavpn servers [-h | --help] [-v | --verbose] [-c | --cache]

List of options:
  -h | --help          Displays help on commandline options.
  -v | --verbose       Verbose mode.
  -c | --cache         From local cache.
```

This operation can be done using the cache or interacting with the server. By
default the cache is not used.

Verbose mode shows more data for each server. Without verbose mode the output
is like this (here the first 10 lines):

```
% mozillavpn servers | head -n 10
- Country: Australia (code: au)
  - City: Melbourne (mel)
    - Server: au3-wireguard
    - Server: au4-wireguard
  - City: Sydney (syd)
    - Server: au10-wireguard
    - Server: au11-wireguard
    - Server: au12-wireguard
    - Server: au13-wireguard
    - Server: au14-wireguard
```

In verbose mode, this is the output:

```
% mozillavpn servers -v | head -n 20
- Country: Australia (code: au)
  - City: Melbourne (mel)
    - Server: au3-wireguard
        ipv4 addr-in: 103.231.88.2
        ipv4 gateway: 10.64.0.1
        ipv6 addr-in: 2407:a080:3000:12::a03f
        ipv6 gateway: fc00:bbbb:bbbb:bb01::1
        public key: Rzh64qPcg8W8klJq0H4EZdVCH7iaPuQ9falc99GTgRA=
    - Server: au4-wireguard
        ipv4 addr-in: 103.231.88.18
        ipv4 gateway: 10.64.0.1
        ipv6 addr-in: 2407:a080:3000:11::a04f
        ipv6 gateway: fc00:bbbb:bbbb:bb01::1
        public key: Y3yfOLqFCCioEzEV8kB+qD3euGIlc1vCHOzclkSyXSg=
  - City: Sydney (syd)
    - Server: au10-wireguard
        ipv4 addr-in: 89.44.10.114
        ipv4 gateway: 10.64.0.1
        ipv6 addr-in: 2001:ac8:84:33::a10f
        ipv6 gateway: fc00:bbbb:bbbb:bb01::1
```

## Selecting a server

I can select a server using ‘select’ and the server’s hostname. For instance:

```
% mozillavpn select au4-wireguard
```

No output is shown, but I can check the current server using status:

```
% mozillavpn status -c | grep Server
Server country code: au
Server country: Australia
Server city: Melbourne
Activating the VPN tunnel
```

The activation is done using ‘activate’:

```
% mozillavpn activate
The VPN tunnel is now active
```

I cannot activate the VPN tunnel more than once:

```
% mozillavpn activate
The VPN tunnel is already active
```

## Deactivating the VPN tunnel

The deactivation is done in the same way, using the command ‘deactivate’:

```
% mozillavpn deactivate
The VPN tunnel is now inactive
```

I cannot deactivate something already deactivated:

```
% mozillavpn deactivate
The VPN tunnel is already inactive
```

## Device removal

A device can be removed using the command ‘device’:

```
% mozillavpn device        
usage: mozillavpn device <device_id>

The list of <device_id> can be obtained using: 'mozillavpn status'
```

## Missing parts:

- Settings
- Opening links (terms or services, manage the account, etc)
