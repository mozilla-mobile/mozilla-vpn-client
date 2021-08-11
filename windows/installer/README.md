# Installer
A WiX script for packaging up all binaries and supporting files in a convenient MSI file, suitable for further distribution.

## Building
A build script is included in this folder, which creates an installer for each platform and handles downloading all dependencies, such as [WiX](https://wixtoolset.org/) and [Wintun](https://www.wintun.net/). Don't attempt to run the scripts without building the [tunnel](../tunnel), followed by the [Mozilla VPN client](../../src) for x64 target platform first.

If the build succeeds, `x64/MozillaVPN.msi` should appear.
