# Build for Linux

## Activate conda

`conda activate vpn`

See [here](./index.md#conda) for conda environment instructions.

## Get Qt

Download the online installer from the official QT website: https://www.qt.io/download-qt-installer

Our usage is covered by the QT open source license.

While you will need to register with an email address, it will be free.

During the install, there will be many components available.
Install all of Qt version 6.2.4 and you'll have everything you need.

## Other dependencies

On Ubuntu, apt install the following dependencies

* libsecret-1 (>=0.18)
* libcap-dev (>=1:2.32)
* libpolkit-gobject-1-dev
* wireguard (>=1.0.20200319)
* wireguard-tools (>=1.0.20200319)

# Build and install

Make the build directory

    mkdir build

Configure

    cmake -S . -B build -DCMAKE_PREFIX_PATH=$(Qt install path)/gcc_64/lib/cmake/

Compile

    cmake --build build -j$(nproc)

Install

    sudo cmake --install build

# Run

After the installation, you can run the app with

    mozillavpn


Alternatively, you can use two terminals to run the daemon manually and separately

    sudo mozillavpn linuxdaemon
    mozillavpn

# Building for Flatpak

The Mozilla VPN client supports building and distribution via flatpak, and is
available on Flathub at [org.mozilla.vpn](https://flathub.org/apps/org.mozilla.vpn).

The flatpak manifests for the latest stable release can be found at
[flathub/org.mozilla.vpn](https://github.com/flathub/org.mozilla.vpn), and this
project also contains a manifest for local development in the `linux/flatpak`
directory.

To build the flatpak package, start by installing the `org.flatpak.Builder` app:

```
   flatpak install -y flathub org.flatpak.Builder
```

Add the Flathub repo user-wide:

```
   flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
```

Then build the project manifest and install it:

```
   flatpak run org.flatpak.Builder --force-clean --sandbox --user --install --install-deps-from=flathub --ccache build-flatpak linux/flatpak/org.mozilla.vpn.yml
```

Once installed, the Mozilla VPN client can be run using the `org.mozilla.vpn`
app identifier:

```
   flatpak run org.mozilla.vpn
```

