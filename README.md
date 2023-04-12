# Mozilla VPN

>_One tap to privacy
Surf, stream, game, and get work done while maintaining your privacy online.
Whether you’re traveling, using public WiFi, or simply looking for more online
security, we will always put your privacy first._

See: https://vpn.mozilla.org

## Getting Involved

We encourage you to participate in this open source project. We love Pull
Requests, Bug Reports, ideas, (security) code reviews, or any other kind of
positive contribution.

Before you attempt to make a contribution please read the [Community
Participation
Guidelines](https://www.mozilla.org/en-US/about/governance/policies/participation/).

Here are some useful links to start:

* [View open
  issues](https://github.com/mozilla-mobile/mozilla-vpn-client/issues)
* [View open pull
  requests](https://github.com/mozilla-mobile/mozilla-vpn-client/pulls)
* [File an
   issue](https://github.com/mozilla-mobile/mozilla-vpn-client/issues/new/choose)
* [File a security
  issue](https://bugzilla.mozilla.org/enter_bug.cgi?product=Mozilla%20VPN)
* Join the [matrix
  channel](https://chat.mozilla.org/#/room/#mozilla-vpn:mozilla.org)
* [View the wiki](https://github.com/mozilla-mobile/mozilla-vpn-client/wiki)
* [View the support
  docs](https://support.mozilla.org/en-US/products/firefox-private-network-vpn)
* Localization happens on
  [Pontoon](https://pontoon.mozilla.org/projects/mozilla-vpn-client/).

### Pre-commit formatting hook

If you want to submit a pull-request, please, install the clang format
pre-commit hook that lints code.

1. The standard conda environment includes the required clang-format libraries.
If the conda environment is *not* being used, a clang-format library will need
to manually installed. (For example, using Homebrew on macOS:
`brew install clang-format`.)

2. The linter will need to know where to find the `clang-format-diff.py` file, so
`CLANG_FORMAT_DIFF` must be exported. On a standard conda installation:
`export CLANG_FORMAT_DIFF=$(find ~/miniconda3/pkgs -name clang-format-diff.py)`

3. Install the pre-commit hook: `./scripts/git-pre-commit-format install`

## Checking out the source code

```bash
git clone https://github.com/mozilla-mobile/mozilla-vpn-client.git
cd mozilla-vpn-client
git submodule update --init
```

## Minimum build requirements

This list is not comprehensive. Different platforms may require other tools to be installed.
Check out the platform section for more information on your specific platform.

- [C++20](https://en.cppreference.com/w/cpp/20)
- [Qt6](https://www.qt.io/product/qt6)
- [Python 3](https://www.python.org/downloads/)
- [Cmake > 3.16](https://cmake.org/install/)
- [Rust](https://www.rust-lang.org/tools/install)
- [Go](https://go.dev/)

### Installing Qt6

Qt6 can be installed in a number of ways:

- Download a binary package or the installer from the official QT website:
  https://www.qt.io/download-qt-installer - this is the recommended way for
  Android and iOS builds. (Our usage is covered by the QT open source license.
  While you will need to register with an email address, it will be free.)
  During the install, there will be many components available. Install:
  - QT version (recommend most recent LTS):
    - WebAssembly (TP)
    - macOS (if you'll be doing macOS work)
    - Android (if you'll be doing Android work)
    - iOS (if you'll be doing iOS work)
    - QT 5 Compatibility Module
    - Additional Libraries
       - Qt Networking Authorization
       - Qt WebSockets
    - Qt Debug Information Files
  - Developer and Designer Tools
    - CMake
    - Ninja
- Use a package manager. For instance, we use
  [aqt](https://github.com/miurahr/aqtinstall) for WASM builds.
- Compile Qt6 (dynamically or statically). If you want to choose this path, you
  can use our bash script for macOS and Linux:
```bash
./scripts/utils/qt6_compile.sh </qt6/source/code/path> </destination/path>
```
- Grab a Static Qt-Build used in Mozilla CI:
  - [iOS](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-ios.latest/artifacts/public%2Fbuild%2Fqt6_ios.zip)
  - [MacOS](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-mac.latest/artifacts/public%2Fbuild%2Fqt6_mac.zip)
  - [Windows](https://firefox-ci-tc.services.mozilla.com/api/index/v1/task/mozillavpn.v2.mozillavpn.cache.level-3.toolchains.v3.qt-win.latest/artifacts/public%2Fbuild%2Fqt6_win.zip)

### Installing Python 3

[Python](https://www.python.org/) >= 3.6 is required. You also need to install
a few python modules using [pip](https://pypi.org/):

```bash
pip install -r requirements.txt --user
```

(`pip3` may need to be substituted for `pip` in the above line.)

### Installing CMake

There are many ways to install [CMake](https://cmake.org).

On macOS, it is easy to do with [Homebrew](https://brew.sh/). After
[installing Homebrew](https://brew.sh/#install), run:
```bash
brew install cmake
```

### Installing Rust

[Rust](https://www.rust-lang.org/) is required for desktop builds (macOS, Linux
and Windows). See the official rust documentation to know how to install it.

### What's next?

We support the following platforms: Linux, Windows, macOS, iOS, Android and
WASM. Each one is unique and it has a different section in this document.

## How to build from source code for Desktop

On desktop platforms, such as Windows, Linux and macOS, we build the Mozilla VPN
using CMake, and as long as the required dependencies can be located in your
PATH the build process is effectively the same on each of the supported platform.

1. Start by creating a build directory and using `cmake` to generate the Makefiles.

```bash
mkdir build && cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr
```

This generation step can be augmented by providing variable definitions on the
command line to adjust features, or help CMake locate its dependencies. (Using these
may look like `cmake -S . -B build -GXcode -DCMAKE_PREFIX_PATH=/Users/[your account]/Qt/6.2.4/macos/lib/cmake`)
The following variables may be of use:

- `CMAKE_PREFIX_PATH=<Qt install path>/lib/cmake`: can be set if CMake is unable
   to locate a viable Qt installation in your `PATH`. (This may be an error message
   like `Unknown CMake command "qt_add_executable”.`)
- `CMAKE_BUILD_TYPE=Release`: can be set to generate a release build, otherwise a
  Debug build is generated by default.
- `BUILD_TESTING=ON`: can be set to build, and execute the unit tests using `CTest`
- `BUILD_ID=<string>`: sets the build identifier that will be embedded into the project. If
  left unset, this will generate a timestamp when configuring the Makefiles.

2. Once the makefiles have been generated, the next step is to compile the source code:
```bash
cmake --build build -j$(nproc)
```

The following sections go into further detail for each of the supported platforms.

### How to build from source code for Linux

We have tested Mozilla VPN on Ubuntu, Fedora, and Arch Linux but in this
document, we focus on Ubuntu only.

1. On Ubuntu, the compilation of MozillaVPN is relatively easy. In addition to
what we wrote before, you also need the following dependencies:

- libpolkit-gobject-1-dev >= 0.105
- wireguard >= 1.0.20200513
- wireguard-tools >= 1.0.20200513
- resolvconf >= 1.82
- golang >= 1.13
- cmake >= 3.16

2. **Optional**: In case you want to change the shaders, you must regenerate
them:
```bash
./scripts/utils/bake_shaders.sh
```

3. Create a build directory, and configure the project for building using `cmake`.
```bash
mkdir build && cmake -S . -B build
```

If you are using a build of Qt that was not installed by your operating system,
you may need to tell `cmake` where it is located by specifying the `CMAKE_PREFIX_PATH`
during configuration:
```bash
mkdir build && cmake -S . -B build -DCMAKE_PREFIX_PATH=<Qt install path>/lib/cmake/
```

4. Compile the source code:
```bash
cmake --build build -j$(nproc)
```

5. Installation:
```bash
sudo cmake --install build
```

6.  After the installation, you can run the app simply running:
```bash
mozillavpn
```

Alternatively, you can use two terminals to run the daemon manually and
separately e.g.

```bash
sudo mozillavpn linuxdaemon
mozillavpn
```

mozillavpn linuxdaemon needs privileged access and so if you do not run as
root, you will get an authentication prompt every time you try to reconnect the
vpn.

### How to build from source code on macOS

There are two ways to build the project on macOS - Through the terminal or with Xcode.
For both, you must install a copy of the VPN application in the main macOS Applications
folder so that the daemon is available for your debug build.
[Any recent build](https://archive.mozilla.org/pub/vpn/releases/) should suffice.
(If major changes were made to the daemon since the last release, you may need to use
a copy of the debug build for the macOS Applications folder.)

#### Building from terminal

1. On macOS, we compile the app using
[Xcode](https://developer.apple.com/xcode/) version 12 or higher.

2. You also need to install go >= v1.18. If you don't have it done already,
download go from the [official website](https://golang.org/dl/).

3. Create a build directory, and configure the project for building using `cmake`.
```bash
# Run this next line from the main project folder
mkdir build && cmake -S . -B build
```

Some variables that might be useful when configuring the project:
 - `CMAKE_PREFIX_PATH=<Qt install path>/lib/cmake`: can be set if CMake is unable to
   locale a viable Qt installation in your path.
 - `CODE_SIGN_IDENTITY=<Certificate Identity>`: can be set to enable code signing during
   the build process.
 - `INSTALLER_SIGN_IDENTITY=<Certificate Identity>`: can be set to enable signing of the
   installer package.
 - `BUILD_OSX_APP_IDENTIFIER=<App Identifier>`: can be set to change the application bundle
   identifier. This defaults to `org.mozilla.macos.FirefoxVPN` if not set.
 - `BUILD_VPN_DEVELOPMENT_TEAM=<Development Team ID>`: can be set to change the development
   team used for Xcode certificates. This defaults to `43AQ936H96` if not set.
 - `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"`: can be set to produce a universal binary that
   will run on both Intel and Apple silicon devices (experimental).

4. Compile the source code:
```bash
cmake --build build -j$(nproc)
```

This will produce the application bundle in `build/src/Mozilla VPN.app`.

> **Note**: to open the application from the command line, run `open -n build/src/Mozilla\ VPN.app`.

> **Note**: To enable the VPN built from source you either need to install and run the released VPN client in the background, or run the daemon manually from the terminal. 
To run the daemon from terminal go to `/Users/mozilla/Desktop/mozilla-vpn-client/build/src/Mozilla VPN.app/Contents/MacOS` and run `sudo ./Mozilla\ VPN macosdaemon`.

5.  **Optional**: To build the installer package, we can specify the `pkg` target when
building:
```bash
cmake --build build --target pkg
```

This will produce an unsigned installer package at `build/macos/pkg/MozillaVPN-unsigned.pkg`
and a signed installer at `build/macos/pkg/MozillaVPN-signed.pkg` if a valid installer
signing identity was provided in the `INSTALLER_SIGN_IDENTITIY` variable at configuration
time.

#### Building with Xcode

Before you start this process, open Xcode, go to settings, accounts, and sign in with your
Apple ID.

In some circumstances, you may wish to use Xcode to build the Mozilla VPN in order to
access cloud-managed signing certificates. In such circumstances, this can be enabled
by using the `-GXcode` command line option:

```bash
mkdir build && cmake -S . -B build -GXcode
```

This will generate an Xcode project file at `build/Mozilla VPN.xcodeproj` which can be opened
by Xcode:

```bash
open build/Mozilla\ VPN.xcodeproj
```

Once Xcode has opened the project, building is as simple as selecting the `mozillavpn` target
and starting the build from the `Product->Build For->Testing` menu.

*Note*: some developers have experienced that Xcode reports that `go` isn't
available and so you can't build the app and dependencies in Xcode. (This may show up as build
errors like `Run custom shell script ‘Generate extension/CMakeFiles/cargo_mozillavpnnp’ \ No
such file or directory \ Command PhaseScriptExecution failed with a nonzero exit code`) In this
case, a workaround is to symlink `go` into Xcode directory as follows:

* Make sure go is 1.18+: `go version`
* Find the location of go binary `which go` example output `/usr/local/go/bin/go`
* Symlink e.g.
```bash
cd /Applications/Xcode.app/Contents/Developer/usr/bin/
sudo ln -s /usr/local/go/bin/go
# You may also need to symlink Cargo and Rust
sudo ln -s $(which cargo)
sudo ln -s $(which rustc)
```

This step needs to be executed each time Xcode updates.

#### Building using Conda on MacOS

We provide a Conda env for easy Setup.
Prerequisites:
- Have miniconda installed.

```bash
$ conda env create -f env.yml
$ conda activate VPN
```
 - Get a copy of a MacOS-SDK (every X-Code install ships this, or you can find it on the internet :) )
    - Set `SDKROOT` to the target SDK: `export SDKROOT=$(xcrun --sdk macosx --show-sdk-path)`
    - Add it to the conda env via: `conda env config vars set SDKROOT=<>`
    - Default Paths where you probably find your SDK:
      - Default Xcode-command-line tool path: `/Library/Developer/CommandLineTools/SDKs/MacOSX.<VersionNumber>.sdk`
      - Default Xcode.app path: `/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk`
 - Get a Build of QT (See: [Installing Qt6](#Installing-Qt6) )

You are now ready to build!

```bash
(vpn) $ mkdir build && cmake -S . -B build -DCMAKE_PREFIX_PATH=${Path to QT}/lib/cmake
(vpn) $ cmake --build build
```

### How to build from source code for iOS

To build the project on iOS, we use `cmake` to configure the Xcode project files, which we
can then open and build via the Xcode IDE.

> **Note**: Due to lack of low level networking support, it is not possible to turn on
> the VPN from the iOS simulator in Xcode.

1. On iOS, we compile the app using
[Xcode](https://developer.apple.com/xcode/) version 12 or higher and [Qt](https://www.qt.io/download)
version 6.2.4.

2. Ensure rust targets for iOS development are installed.
```bash
rustup target add x86_64-apple-ios aarch64-apple-ios
```

3. We use `qt-cmake` from the Qt installation to configure the Xcode project.
```bash
mkdir build-ios
/Users/example/Qt/6.2.4/ios/bin/qt-cmake . -B build-ios -GXcode
```

Some variables that might be useful when configuring the project:
 - `BUILD_ADJUST_SDK_TOKEN=<SDK Token>`: can be set to enable the use of the Adjust telemetry
   and attribution data collection.
 - `BUILD_IOS_APP_IDENTIFIER=<App Identifier>`: can be set to change the application bundle
   identifier. This defaults to `org.mozilla.ios.FirefoxVPN` if not set.
 - `BUILD_VPN_DEVELOPMENT_TEAM=<Development Team ID>`: can be set to change the development
   team used for Xcode certificates. This defaults to `43AQ936H96` if not set.

4. Open the generated Xcode project with `open build-ios/Mozilla\ VPN.xcodeproj`.

5. Select the `mozillavpn` target and `Any iOS Device (arm64)` as the build configuration
for iOS devices, or select any of the simulation targets when building for the simulator.

6. Click on the Play button to start building and signing of the Mozilla VPN app.

### How to build from source code for Android

1. You need to install go >= v1.18. If you don't have it done already, download
it from the [official website](https://golang.org/dl/).

2. Follow the [Getting started](https://doc.qt.io/qt-6/android-getting-started.html) page.

3. Set the `QT_HOST_PATH` environment variable to point to the location of the `androiddeployqt` tool  -- minus the `/bin` suffix i.e. if `$(which androiddeployqt)` is `$HOME/Qt/6.2.4/gcc_64/bin/androiddeployqt`, `QT_HOST_PATH` is `$HOME/Qt/6.2.4/gcc_64/`.

4. Set the `ANDROID_SDK_ROOT` and `ANDROID_NDK_ROOT` environment variables,
to point to the Android SDK and NDK installation directories. Required NDK versions: 23.1.7779620 and 21.0.6113669.

5. Add the Android NDK llvm prebuilt tools to your `PATH`. These are located under the Android NDK installation
directory on `${ANDROID_NDK_ROOT}/toolchains/llvm/prebuilt/*/bin`.

6. Install the Rust Android targets `rustup target add x86_64-linux-android i686-linux-android armv7-linux-androideabi aarch64-linux-android`.

7. Build the apk
```bash
./scripts/android/cmake.sh -d </path/to/Qt6/> -A <architecture> <debug|release>
```
Add the Adjust SDK token with `-a | --adjust <adjust_token>`.

Valid architecture values: `x86`, `x86_64`, `armeabi-v7a` `arm64-v8a`, by default it will use all.

8. The apk will be located in
`.tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk`

9. Install with adb on device/emulator
```bash
adb install .tmp/src/android-build/build/outputs/apk/debug/android-build-debug.apk
```

### How to build from source code for Windows

1. For Windows, there are a few extra dependencies to install:

- perl: http://strawberryperl.com/
- nasm: https://www.nasm.us/
- Visual Studio 2019: https://visualstudio.microsoft.com/vs/
  - Select the `Desktop development with C++` and `Python development` workloads.
- OpenSSL: https://www.openssl.org/source/
  - On windows you can choose to bundle OpenSSL when installing python. Skip this step if you have done so.
- Go (go version >= v1.18 required): https://golang.org/dl/

We strongly recommend using CMake version 3.21 or later when building with Visual
Studio. Earlier versions of CMake have bugs that can cause the build to hang.

It is also recommended to use the `x64 Native Tools Command Prompt for VS 2019` for CLI builds on Windows.

2. Create a build directory, and configure the project for building using `cmake`.
```bash
mkdir build && cmake -S . -B build
```

3. Compile the source code.
```bash
cmake --build build --config Release
```

4.  **Optional**: To build the MSI installer package, we can specify the `msi` target
when building:
```bash
cmake --build build --config Release --target msi
```
5.  **Optional**: To build and debug through the VS 2019 UI, follow these steps:
    - Open VS2019
    - From the top menu, select File -> Open -> CMake
    - Choose the top-level `CMakeLists.txt` of the VPN project.
    - Choose `x64-Debug` as the build config
    - Choose `src/Mozilla VPN.exe` as the startup item.
    - Click on the green play button to launch the client attached to a debugger.

### How to build from source code for WASM

Mozilla VPN can be used as a WASM module to test the UI, the localization and
to simulate issues. Take a look at our WASM build
[here](https://mozilla-mobile.github.io/mozilla-vpn-client/).

To build the app as a WASM module, you must install
[emscripten](https://emscripten.org/) and Qt6 for wasm.

Read the following pages to know more:
- https://emscripten.org/docs/getting_started/index.html
- https://doc.qt.io/qt-6/wasm.html

When you are ready, create a build directory and configure the project for
building using `qt-cmake` + `cmake`:
```bash
mkdir build && qt-cmake cmake -S . -B build
```

Compile the source code:
```bash
cmake --build build -j$(nproc)
```

## Testing

### Unit tests

When built for any one of the desktop platforms, this project will also generate
a suite of unit tests.

The tests are built manually specifying the `build_tests` target.

```bash
cmake --build build --target build_tests -j $(nproc)
```

Once built, you can run them with `ctest` as follows:

```
ctest --test-dir build -j $(nproc) --output-on-failure
```

### Running the functional tests

**New build required**: Functional tests require a dummy build of the application, which is not
built by default. To build the `dummyvpn` target, in the root folder of this repository run:

```
cmake --build build -j$(nproc) --target dummyvpn
```

This will create a dummy build under the `tests/dummyvpn` folder. To run the functional
tests against this build, make sure the `MVPN_BIN` environment variable is set:

```
export MVPN_BIN=$(pwd)/build/tests/dummyvpn/dummyvpn
```

**Other dependencies**:
* Install node (if needed) and then `npm install` to install the testing
  dependencies
* Compile the testing addons: `./scripts/addon/generate_all_tests.py`
* Make a .env file and place it in the root folder for the repo. It should include:
 * `MVPN_BIN` (location of compiled mvpn binary. This must be a dummy binary, see note above.)
 * `ARTIFACT_DIR` - optional (directory to put screenshots from test failures)
 * Sample .env file:
  ```
  export PATH=$PATH:~/Qt/6.2.4/macos/bin:$PATH
  export QT_MACOS_BIN=~/Qt/6.2.4/macos/bin
  MVPN_API_BASE_URL=http://localhost:5000
  MVPN_BIN=dummybuild/src/mozillavpn
  ARTIFACT_DIR=tests/artifact
  ```

**To run a test**: from the root of the project: `npm run functionalTest path/to/testFile.js`. To run, say, the authentication tests: `npm run functionalTest tests/functional/testAuthenticationInApp.js`.

## Developer Options and staging environment

To enable the staging environment, open the `Get Help` window, and click on the
`Get Help` text 6 times within 10 seconds to unlock the Developer Options menu.
On this menu, you can enable on the `Staging Server` checkbox to switch to the
staging environment. A full restart of the VPN will be required for this option
to take effect.

## Inspector

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

## Glean

[Glean](https://docs.telemetry.mozilla.org/concepts/glean/glean.html) is a
Mozilla analytics & telemetry solution that provides a consistent
experience and behavior across all of Mozilla products.

When the client is built in debug mode, pings will have the [app channel](app-channel) set to
`debug`. Additionally, ping contents will be logged to the client
logs.

When the client is in staging mode, but not debug mode, pings will have the
[app channel](app-channel) set to `staging` which allows for filtering between staging
and production pings through the `client_info.app_channel` metric present in all pings.

[app-channel]: https://mozilla.github.io/glean/book/reference/general/initializing.html?highlight=app%20channel#gleaninitializeconfiguration

### A note on Glean embedding

Qt only accepts `major.minor` versions for importing. So if, for example,
you're embedding glean v0.21.2 then it will still, for Qt's purpose, be v0.21.

### Working on tickets with new Glean instrumentation

If you are responsible for a piece of work that adds new Glean instrumentation you will need to do a data review.
Following is the recommended process along with some pointers.

> The data review process is also described here: https://wiki.mozilla.org/Data_Collection

The basic process is this:

* Implement the new instrumentation. Refer to [the Glean book](https://mozilla.github.io/glean/book/user/metrics/adding-new-metrics.html) on how to do that.
* When adding or updating new metrics or pings, the [Glean YAML files](https://github.com/mozilla-mobile/mozilla-vpn-client/tree/main/glean) might need to be updated.
  When that is the case a new data-review must be requested and added to the list of data-reviews for the updated/added instrumentation.
  When updating data-review links on the YAML files, these are the things to keep in mind:
  * Include a link to the *GitHub* bug that describes the work, this must be a public link;
  * Put "TBD" in the `data_reviews` entry, that needs to be updated *before* releasing the new instrumentation and ideally before merging it;
  * Think about whether the data you are collecting is technical or interaction, sometimes it's both. In that case pick interaction which is a higher category of data. (See more details on https://wiki.mozilla.org/Data_Collection);
* Open a **draft** PR on GitHub;
* Fill out the data-review[^1] form and request a data-review from one of the [Mozilla Data Stewards](https://wiki.mozilla.org/Data_Collection)[^2].
  That can be done by opening a Bugzilla ticket or more easily by attaching the questionnaire as a comment on the PR that implements the instrumentation changes.
  For Bugzilla, there is a special Bugzilla data review request option and for GitHub it's enough to add the chosen data steward as a reviewer for the PR.
* The data-review questionnaire will result in a data review response. The link to that response is what should be added to the `data_review` entry on the Glean YAML files.
  It must be a public link.

> Note:
> - It is **ok** for a reviewer to review and approve your code while you're waiting for data review.
> - It is **not** ok to release code that contains instrumentation changes without a data review r+. It is good practice not to merge code that does not have a data review r+.

[^1]: The data-review questionnaire can be found at https://github.com/mozilla/data-review/blob/main/request.md. That can be copy pasted and filled out manually. However,
since the VPN application uses Glean for data collection developers can also use the [`glean_parser data-review`](https://mozilla.github.io/glean_parser/) command,
which generates a mostly filled out data-review questionnaire for Glean users. The questionnaire can seem quite intimidating, but don't panic.
First, look at an old data-review such as https://github.com/mozilla-mobile/mozilla-vpn-client/pull/4594.
Questions 1, 2, 3 an 10 are the ones that require most of your attention and thought.
If you don't know the answers to these questions, reach out to Sarah Bird or the product manager so you can answer these with full confidence.
[^2]: Feel free to ping any of the data-stewards. If the collection is time sensitive consider pinging all data-stewards directly on the [data-stewards](https://matrix.to/#/#data-stewards:mozilla.org) matrix channel.

## Status

[![Unit Tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_unit.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_unit.yaml)
[![Lottie Tests](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_lottie.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/test_lottie.yaml)
[![Linters (clang, l10n)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linters.yaml)
[![Linux Packages](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linux.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/linux.yaml)
[![WebAssembly](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/wasm.yaml/badge.svg)](https://github.com/mozilla-mobile/mozilla-vpn-client/actions/workflows/wasm.yaml)
