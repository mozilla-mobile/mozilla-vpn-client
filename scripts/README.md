# Script folder

This folder contains several scripts useful for the compilation, the tests, the
execution of the Mozilla VPN client.

# Test scripts

The `tests` folder contains anything related to test execution. These scripts
can be used to run tests locally or via the CI.

- ./tests/qml_tests.sh - Runs QML tests (Linux, MacOS)
- ./tests/unit_tests.sh - Runs unit tests (Linux, MacOS)
- ./tests/lottie_tests.sh - Runs lottie tests (Linux, MacOS)
- ./tests/functional_tests.ps1 - run functional tests on windows
- ./tests/functional_tests.sh - run functional tests

# Android-specific scripts

- ./android/package.sh - compile the client for android. See the main README.md file.
- ./android/sign.sh - sign the APK

# Linux-specific scripts

- ./linux/build.sh - compile the client for linux. See the main README.md file.
- ./linux/ppa_script.sh - upload ubuntu packages to the PPA
- ./linux/script.sh - compile the client and upload it to the PPA
- ./linux/utils/commons.sh - common functions for cross-platform scripts

TODO: unify build.sh and script.sh (or remove build.sh)

# MacOS-specific scripts

- ./macos/apple_compile.sh - compile the client for iOS or macOS. See the main README.md file.
- ./macos/utils/xcode_patcher.rb - tool to patch xcode project
- ./macos/utils/commons.sh - common functions for cross-platform scrips
- ./macos/import_pkg_resources.py - configure resources for the PKG generation
- ./macos/scope_only_change.sh - runs tests when needed
- ./macosx_build.sh

TODO:
1. scope_only_change.sh should be croll-platform! Do we actually use it?
2. remove/merge/move macos_build.sh

# Windows-specific scripts

- ./windows/utils/commons.sh - common functions for cross-platform scrips
- ./windows_build.sh - script for taskcluster
- ./windows_compile.bat - compile the client
- ./openssl_compile.bat - compile openssl
- ./Qt5_static_compile.bat - compile QT5

TODO:
1. move all the windows scripts into the `windows` folder
2. remove/unify windows_build.sh and windows_compile.bat

# Wasm-specific scripts

- ./wasm/compile.sh - compile the client for Wasm. See the main README.md file.

# CI tools

- ./ci/check_qrc.py - check qrc files to avoid duplicate entries and other errors

# Utilities for the compilation

- ./utils/bake_shaders.sh - create the shader resources
- ./utils/commons.sh - common set of functions
- ./utils/generate_glean.py - generate glean resources
- ./utils/generate_i18n_servers.sh - localize city/country names
- ./utils/generate_strings.py - process the string.yaml file and generate resources
- ./utils/import_languages.py - process languages and import them
- ./utils/inspector.py - send commands to the VPN client inspector
- ./utils/qt5_compile.sh - compile qt5 for linux and macos
- ./utils/qt6_compile.sh - compile qt6 for linux and macos

# Clang-format utils

TODO: all of these needs to be moved in subfolders, removed, unified, etc.

- ./apply-format - apply clang-format rules
- ./git-pre-commit-format - configure the pre-commit git hook for clang-formatting

TODO: all of these needs to be moved in subfolders, removed, unified, etc.

# Others

- ./tooltool.py - utility used in taskcluster
