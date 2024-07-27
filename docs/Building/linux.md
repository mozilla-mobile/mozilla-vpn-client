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
* libcap2-bin (>=1:2.32)
* wireguard (>=1.0.20200319)
* wireguard-tools (>=1.0.20200319)

On Fedora, ensure dependencies are installed

    sudo dnf install libsecret-devel libcap-devel wireguard-tools make

# Build and install

Make the build directory

    mkdir build

Configure

    cmake -S . -B build -DCMAKE_PREFIX_PATH=$(Qt install path)/gcc_64/lib/cmake/

> hint about $(Qt install path): If you forgot where you installed it, you can use the following

    sudo updatedb  # if you just installed Qt
    locate -br ^gcc_64$

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
