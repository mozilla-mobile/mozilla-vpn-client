# Script folder

This folder contains several scripts useful for the compilation, the tests, the
execution of the Mozilla VPN client.

# Test scripts

The `tests` folder contains anything related to test execution. These scripts
can be used to run tests locally or via the CI.

# Android-specific scripts

- ./android/package.sh - compile the client for android. See the main README.md file.
- ./android/sign.sh - sign the APK

# Linux-specific scripts

- ./linux/script.sh - compile the client and upload it to the PPA
- ./linux/getdeps.py - extract dependencies from the Debian package control file.

# CI tools

- ./ci/check_qrc.py - check qrc files to avoid duplicate entries and other errors

# Utilities for the compilation

- ./utils/bake_shaders.sh - create the shader resources
- ./utils/commons.sh - common set of functions
- ./utils/generate_strings.py - process the string.yaml file and generate resources
- ./utils/import_languages.py - process languages and import them
- ./utils/inspector.py - send commands to the VPN client inspector
- ./utils/qt6_compile.sh - compile qt6 for Linux and MacOS

# Clang-format utils

- ./apply-format - apply clang-format rules
- ./git-pre-commit-format - configure the pre-commit git hook for clang-formatting

# Addons

- ./addon/build.py - generate a single addon

# Others

- ./tooltool.py - utility used in taskcluster
