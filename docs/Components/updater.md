# How the client updates itself

The client has 2 “force update” mechanisms. Both the `VersionAPI` class and the `Balrog` class can emit `updateRequired` and `updateRecommended` signals.

`Updater.cpp` is what decides which of three systems to use to check the latest version: `Balrog` (uses the service balrog), `VersionAPI` (uses a Guardian API that seems to feed from the product details repository), or `WebUpdater` (which sends the user to a webpage).

- `Balrog`: Windows and macOS use Balrog (controlled by the `MVPN_BALROG` flag defined in the macOS and Windows cmake files).
- `VersionAPI`: It is believed that all other platforms use VersionAPI. (This will be confirmed in [VPN-6836](https://mozilla-hub.atlassian.net/browse/VPN-6836).)
- `WebUpdater`: It is believed that this is unused code, but that will be confirmed (and removed) in [VPN-6836](https://mozilla-hub.atlassian.net/browse/VPN-6836).
