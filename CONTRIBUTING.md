# Contributing

Discussions happen in [#mozilla-vpn:mozilla.org](https://chat.mozilla.org/#/room/#mozilla-vpn:mozilla.org) Matrix Channel.

1. [Issues marked as `good-first-bug`](https://github.com/mozilla-mobile/mozilla-vpn-client/labels/good%20first%20issue) are self-contained enough that a contributor should be able to work on them.
2. Issues are considered not assigned, until there is a PR linked to them.


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

To run the tests, we must set the `MVPN_BIN` environment variable and point it to the Mozilla
VPN binary.

```
# Linux:
export MVPN_BIN=$(pwd)/build/mozillavpn
# macOS:
export MVPN_BIN=$(pwd)/build/src/Mozilla\ VPN.app/Contents/MacOS/Mozilla\ VPN
# Others:
export MVPN_BIN=$(pwd)/build/src/mozillavpn
```

The functional tests also require the testing addons to be built. This sub project can be found
at `tests/functional/addons` and can be built as follows:

```
cmake -B build-addons/ -S tests/functional/addons
cmake --build build-addons/
export MVPN_ADDONS_PATH=build-addons/
```

**Other dependencies**:
* Install node (if needed) and then `npm install` to install the testing
  dependencies
* Make a .env file and place it in the root folder for the repo. It should include:
 * `ARTIFACT_DIR` - optional (directory to put screenshots from test failures)
 * Sample .env file:
  ```
  export PATH=$PATH:~/Qt/6.10.1/macos/bin:$PATH
  export QT_MACOS_BIN=~/Qt/6.10.1/macos/bin
  MVPN_API_BASE_URL=http://localhost:5000
  ARTIFACT_DIR=tests/artifact
  ```

**To run a test**: from the root of the project: `npm run functionalTest path/to/testFile.js`. To run, say, the authentication tests: `npm run functionalTest tests/functional/testAuthenticationInApp.js`.

(Functional tests are run against the production build of the application by providing the
`--testing` flag to the command line when running the app. This option switches the client in to staging mode, and
enables mocking for platform and backend features necessary to facilitate automated testing.
This flag is added automatically when setupVpn.js starts the VPN, which is done for all functional tests.)

## Developer Options and staging environment

To enable the staging environment, open the `Help` window, and click on the
`Help` title text 6 times within 10 seconds to unlock the Developer Options menu.
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

## CPP Notes

We have the includes following these guidelines:
- first the internal headers
- after an empty line, the external headers
- each block (internal and external headers) alphabetic sorted
- if there is a corresponding header for the cpp class, that goes on top. e.g. for file wgutils.cpp, wgutils.h (even if not alphabetic sorted)
