# Build for MacOS

## Xcode

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

## Activate conda

    conda env create -f env.yml
    conda activate vpn

See [here](./index.md#conda) for conda environment instructions.

Install extra conda packages

    ./scripts/macos/conda_install_extras.sh

Your Xcode install comes with a copy of the MacOS-SDK.
We need to tell the conda environment where to find it.

Find the sdk path

    xcrun --sdk macosx --show-sdk-path

If xcrun didn't work, default paths where you probably find your SDK:
 * Default Xcode-command-line tool path: `/Library/Developer/CommandLineTools/SDKs/MacOSX.<VersionNumber>.sdk`
 * Default Xcode.app path: `/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk`

Add it to the conda env

    conda env config vars set SDKROOT=$SDK_PATH

Reactivate your conda env

    conda activate vpn

You can view your set variables

    conda env config vars list

The variable config step only needs to be done once.
When you next want to start building the VPN, all you need to do is activate your conda environment (`conda activate vpn`).

## Get Qt

Get a static build of Qt made built in our CI.

https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-macos-6.6.latest/artifacts/public%2Fbuild%2Fqt6_mac.zip

Unzip the folder and remember the location for the configure step.

# Build

Make the build directory

    mkdir build-mac

Configure

    cmake -S . -B build-mac -DCMAKE_PREFIX_PATH=(Qt unzipped path)/macos/lib/cmake/

Compile

    cmake --build build-mac -j (number of processes to use e.g. 8)

# Run

After building, you can run the app with

    open ./build-mac/src/Mozilla\ VPN.app

However, opening as above will not give you log output and will not let you
connect to the VPN.
To overcome this, run two terminals.
In the first, run

    sudo ./build-mac/src/Mozilla\ VPN.app/Contents/MacOS/Mozilla\ VPN macosdaemon

In the second, run

    ./build-mac/src/Mozilla\ VPN.app/Contents/MacOS/Mozilla\ VPN

For most dev tasks (everything except working on the macosdaemon), you can leave the first terminal with the `macosdaemon` running and rebuild your client in the second with your new changes and test them.

# Building the installer

Use the `--target pkg` to build the MacOS installer.

This will produce an unsigned installer package at `build-mac/macos/pkg/MozillaVPN-unsigned.pkg`
and a signed installer at `build-mac/macos/pkg/MozillaVPN-signed.pkg` if a valid installer
signing identity was provided in the `INSTALLER_SIGN_IDENTITIY` [variable at configuration time](./index.md).

# Building with Xcode

## Configure Xcode build environment

You need to tell Xcode where to find the versions of rust and go that we have installed in our
conda environment.

First find and go to the Xcode `/usr/bin` directory. The `xcrun` step you ran earlier will give you a hint
where to look. It is usually, `/Applications/Xcode.app/Contents/Developer/usr/bin/`.

    cd /Applications/Xcode.app/Contents/Developer/usr/bin/

Now activate your conda environment

    conda activate vpn

Finally link go and rust

    sudo ln -s $(which go)
    sudo ln -s $(which cargo)
    sudo ln -s $(which rustc)

This step needs to be repeated each time Xcode updates.

# Building

Use the same configure command above and add `-GXcode`:

    cmake -S . -B build-mac -DCMAKE_PREFIX_PATH=(Qt unzipped path)/macos/lib/cmake/ -GXcode

This will generate an Xcode project file at `build-mac/Mozilla VPN.xcodeproj` which can be opened
by Xcode:

    open build-mac/Mozilla\ VPN.xcodeproj

Once Xcode has opened the project, select the `mozillavpn` target and start the build.

If you're building with Xcode and want your built VPN client to connect to the VPN network you'll
need to install a release copy of the VPN client which you can download from [here](https://www.mozilla.org/products/vpn/download/).
