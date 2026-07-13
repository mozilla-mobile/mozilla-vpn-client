# Assets

To support theming, there are two different versions of all assets (images and animations). There is one for darker themes, and another for lighter themes.

Within each theme's `theme.js` file, there is a line that controls which set of assets should be used: `color.useDarkAssets = false;` (or `= true`, depending on what is most appropriate). And in the QML code, instead of explicitly setting image assets, code is run to utilize the appropriate asset: `MZAssetLookup.getImageSource("ButtonLoader")`.

**The only assets that do not have both dark and light versions are country flags.** The flag assets referenced in `ServerLabel.qml`, `ServerCountry.qml`, and `SystemTrayNotificationHandler.cpp` are expected to explicitly use assets.

**`MozillaVPN::registerNavigationBarButtons' sets up the SVGs for the bar button items. For reasons lost to time, this is done in C++, not QML.** The C++ `NavigationBarButton` class takes assets for both light and dark modes, and chooses the proper one when the theme changes.

### Adding a new asset
1. Add the asset to the repository.
2. Add the asset in `nebula/ui/resources/CMakeLists.txt` or `src/ui/resources.qrc`, depending on which folder the asset was placed in.
3. Add the asset to the `imageLookup` dictionary within `MZAssetLookup.js`.
4. Use the asset in code (`MZAssetLookup.getImageSource("YourNewName")`).

### Checks and tests
`check_images.py` is run on each PR. It checks 3 things:
1. All image and animation names used in `MZAssetLookup` are unique (there are no duplicates).
2. No QML file uses an explicit image (instead of the proper `MZAssetLookup.getImageSource`), except the expected flags.
3. The image names called in all `MZAssetLookup.getImageSource` actually exist in the `imageLookup` within `MZAssetLookup`.

# Colors

There are three layers to the color files, which build off each other:
1. `colors.js`: Names the explicit hex colors used.
2. Several files in the `color-themes` directory. The name of each file is the name of the theme. This assigns the named hex colors from `colors.js` to variables.
3. `theme-derived.js`: Using the variables from the second step, it names additional variable groups that are derived from the step 2.

When the client uses colors, it merges three files - `colors.js`, the one for the theme, and `theme-derived.js`. There is a hacky piece at the top of `colors.js` and the bottom of `derived-theme.js` to allow this merged file to be read as a JS object. This is well-commented in those files.

### Rules
`check_colors.py` is run on every PR, as part of the linter check. It ensures that colors stay orderly, by checking for bad patterns:
1. `theme.js` files and the `derived-theme.js` file cannot contain color hex codes. (Hex codes only belong in `colors.js`.)
2. QML files cannot contain color hex codes. (Hex codes only belong in `colors.js`.)
3. All theme files (in `color-themes` directory) must have the exact same list of theme-defined colors. (All variables must be available in all themes.)
4. All colors in QML files must be a variable appearing in the theme files or `theme-derived.js`. (Again, no explicit hex codes or color variables - like red20 - should be in QML.)
5. Since iOS widgets are called by a separate system process, it uses colors from `ios/widgetextension/WidgetColors.swift`. To ensure consistency, we check that every color defined in that file is also in `colors.js`, and has a matching RGB value.

These checks keep the color structure flexible, and allows additional themes to be easily added.

