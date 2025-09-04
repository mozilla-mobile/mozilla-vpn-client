# Script folder

This folder contains several scripts useful for the compilation, the tests, the
execution of the Mozilla VPN client.

# Test scripts

The `tests` folder contains anything related to test execution. These scripts
can be used to run tests locally or via the CI.

# Android-specific scripts
See Android build documentation for details on these scripts.

# Linux-specific scripts

- ./linux/script.sh - compile the client and upload it to the PPA
- ./linux/getdeps.py - extract dependencies from the Debian package control file.

# macOS-specific scripts
 See files in `macos` directory

# Windows-specific scripts
 See files in `windows` directory

# CI tools

- ./ci/check_colors.py - check QML files and color files to ensure multiple themes are supported
- ./ci/check_images.py - check QML files to ensure they use the style that allows for different assets for dark mode and light mode. More information available in `assets.md`.
- ./ci/check_jsonschema.py - check JSON schemas for addons, guides, etc.
- ./ci/check_lang_names.py - confirms all language names are set up for translation in the other languages
- ./ci/check_qrc.py - check qrc files to avoid duplicate entries and other errors
- ./ci/update_server_names.py - confirms all server cities and countries are set up for translation

# Utilities for the compilation

- several scripts in `cmake` directory
- ./utils/bake_shaders.sh - create the shader resources
- ./utils/commons.sh - common set of functions
- ./utils/generate_strings.py - process the string.yaml file and generate resources
- ./utils/import_languages.py - process languages and import them
- ./utils/inspector.py - send commands to the VPN client inspector
- ./utils/qt6_compile.sh - compile qt6 for Linux and MacOS
TODO: Add details on several other scripts in the `utils` directory

# Clang-format utils

- ./apply-format - apply clang-format rules
- ./git-pre-commit-format - configure the pre-commit git hook for clang-formatting
- ./run-clang-tidy.py - run clang-tidy

# Addons

- ./addon/build.py - generate a single addon
- ./addon/index.py - TODO: add description

# Others

- ./tooltool.py - utility used in taskcluster
