# Build for MacOS

## Xcode

Install Xcode - https://developer.apple.com/xcode/

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID. If you are a Mozilla developer, this is an Apple ID associated with your LDAP account.

## Activate conda

```
conda env create -f env.yml
conda activate vpn
```

See [here](./index.md#conda) for conda environment instructions.

Your Xcode install comes with a copy of the MacOS-SDK.
We need to tell the conda environment where to find it.

Find the sdk path

```
xcrun --sdk macosx --show-sdk-path
```

If xcrun didn't work, default paths where you probably find your SDK:
 * Default Xcode-command-line tool path: `/Library/Developer/CommandLineTools/SDKs/MacOSX.<VersionNumber>.sdk`
 * Default Xcode.app path: `/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk`

Add it to the conda env

```
conda env config vars set SDKROOT=$SDK_PATH
```

Reactivate your conda env

```
conda activate vpn
```

You can view your set variables

```
conda env config vars list
```

The variable config step only needs to be done once.
When you next want to start building the VPN, all you need to do is activate your conda environment (`conda activate vpn`).

## Get Qt

Get a static build of Qt made built in our CI.

https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-macos-6.6.latest/artifacts/public%2Fbuild%2Fqt6_mac.zip

Unzip the folder and remember the location for the configure step.

# Build

Make the build directory

```
mkdir build-macos
```

Configure

```
cmake -S . -B build-macos -GNinja -DCMAKE_PREFIX_PATH=<Qt unzipped path>/macos/lib/cmake/ -DCODE_SIGN_IDENTITY=<signing identity>
```

Compile

```
cmake --build build-macos
```

# Run

After building, you can run the app with

```
open ./build-macos/src/Mozilla\ VPN.app
```

## Codesigning

Many features of the Mozilla VPN project depend on having a valid code signature. To determine the codesigning identities available to you, you can use the `security` tool on the command line as follows:

```
user@example ~ % security find-identity -v -p codesigning                
  1) AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDD "Apple Development: Jane Doe (XXXXXXXXXX)"
     1 valid identities found
```

Thus, to configure the project to use the above codesigning identity, we can provide the argument `-DCODE_SIGN_IDENTITY=AAAABBBBCCCCDDDDEEEEFFFFAAAABBBBCCCCDDDD`

To distribute a signed application for installation on other machines, an
embedded provisioning profile is also required. The path to the provisioning
profile can be specified with the argument
`-DCODE_SIGN_PROFILE=<path/to/embedded.provisionprofile>`.
When this argument is unset, an attempt will be made to automatically lookup the
provisioning profile from the `~/Library/Developer/Xcode/UserData/Provisioning\ Profiles`
directory.

# Building the installer

Use the `--target macpkg` to build the MacOS installer.

This will produce an macOS installer package at `build-macos/macos/pkg/MozillaVPN.pkg`

# Building with Xcode

## Configure Xcode build environment

You need to tell Xcode where to find the versions of rust and go that we have installed in our
conda environment.

First find and go to the Xcode `/usr/bin` directory. The `xcrun` step you ran earlier will give you a hint
where to look. It is usually, `/Applications/Xcode.app/Contents/Developer/usr/bin/`.

```
cd /Applications/Xcode.app/Contents/Developer/usr/bin/
```

Now activate your conda environment

```
conda activate vpn
```

# Building

Use the same configure command above and use `-GXcode` as the CMake generator:

```
cmake -S . -B build-macos -DCMAKE_PREFIX_PATH=<Qt unzipped path>/macos/lib/cmake/ -GXcode
```

This will generate an Xcode project file at `build-macos/Mozilla VPN.xcodeproj` which can be opened
by Xcode:

```
open build-macos/Mozilla\ VPN.xcodeproj
```

Once Xcode has opened the project, select the `mozillavpn` target and start the build.
