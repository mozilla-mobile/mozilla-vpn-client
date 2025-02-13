# Inspector

The inspector is a debugging tool available only when the staging environment
is activated.

## Tools

The inspector offers a number of tools to help debug and navigate through the VPN client:
* **Shell:** By default the inspector link will take you to the Shell. From there type `help` to see the list of available commands.
* **Logs:** Will constantly output all the app activities happening in real time. This information includes the timestamp, component and message. From the left column you can select which component(s) you'd like to monitor.
* **Network Inspector:** Includes a list of all incoming and outgoing network requests. This is especially helpful when debugging network related issues or monitoring how the app communicates with external components such as the Guardian.
* **QML Inspector:** Allows you to identify and inspect all QML components in the app by mirroring the local VPN client running on your machine and highlighting components by clicking on the QML instance on the right.

![inspector_snapshot](https://user-images.githubusercontent.com/3746552/204422879-0799cbd8-91cd-4601-8df8-0d0e9f7cd887.png)

## To connect on desktop

When running MozillaVPN, go to the [inspector page](
https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/) to interact
with the app. Connect the inspector to the app using the web-socket interface.
On desktop, use `ws://localhost:8765`.

## To connect on iOS

1. On a desktop computer:
    1. Open up Firefox's `about:config` page. Change the `network.websocket.allowInsecureFromHTTPS` setting to `true`.
    2. On the desktop computer, ensure the VPN is turned off.
2. On an iOS device:
    1. Ensure the iOS device is connected to same WiFi network as the desktop computer
    2. Disable cellular on the iOS device (if applicable), so its only connection is the WiFi network.
    3. Note the iOS device's IP address. (Settings app -> Wi-Fi -> ⓘ by network name. See "IP Address" in "IPv4 Address" section)
3. On the desktop computer, go to the [inspector page](
https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/).
On desktop, use `ws://localhost:8765`.
4. On the iOS device, hard quit the Mozilla VPN app (if it is running), and relaunch the app from a cold start. (The actual VPN connection should remain off.)
5. On the Desktop computer, put `ws://[IP addresss]:8765` (example: `ws://192.168.1.131:8765`) in the `Inspector Address` field, click the blue `->` button, and wait for the connection.

Troubleshooting iOS connections:
- There have been reports of having difficulty connecting to iOS using Firefox Nightly. If Nightly is not working, try using the release version of Firefox.
- You may need to attempt a connection to the iOS client shortly after launching the app. If you pause for a while between launching the app and attempting the connection, it may not work.
- You may need to quit and re-launch Firefox after changing the config setting. (Not confident about this.)
- If the iOS device's screen goes to sleep, iOS may kill the websocket connection.
