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

## Building the Client

Moved here -> [Dev-Setup](./docs/dev-setup/index.md)

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
