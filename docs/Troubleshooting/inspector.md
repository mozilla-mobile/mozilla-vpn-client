# Inspector

The inspector is a debugging tool available only when the staging environment
is activated. When running MozillaVPN, go to the [inspector page](
https://mozilla-mobile.github.io/mozilla-vpn-client/inspector/) to interact
with the app. Connect the inspector to the app using the web-socket interface.
On desktop, use `ws://localhost:8765`.

The inspector offers a number of tools to help debug and navigate through the VPN client:
* **Shell:** By default the inspector link will take you to the Shell. From there type `help` to see the list of available commands.
* **Logs:** Will constantly output all the app activities happening in real time. This information includes the timestamp, component and message. From the left column you can select which component(s) you'd like to monitor.
* **Network Inspector:** Includes a list of all incoming and outgoing network requests. This is especially helpful when debugging network related issues or monitoring how the app communicates with external components such as the Guardian.
* **QML Inspector:** Allows you to identify and inspect all QML components in the app by mirroring the local VPN client running on your machine and highlighting components by clicking on the QML instance on the right.

![inspector_snapshot](https://user-images.githubusercontent.com/3746552/204422879-0799cbd8-91cd-4601-8df8-0d0e9f7cd887.png)
