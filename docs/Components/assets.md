# Assets used in app

To support theming, there are two different versions of all assets (images and animations). There is one for darker themes, and another for lighter themes.

Within each theme's `theme.js` file, there is a line that controls which set of assets should be used: `color.useDarkAssets = false;` (or `= true`, depending on what is most appropriate). And in the QML code, instead of explicitly setting image assets, code is run to utilize the appropriate asset: `MZAssetLookup.getImageSource("ButtonLoader")`.

**The only assets that do not have both dark and light versions are country flags.** The flag assets referenced in `ServerLabel.qml`, `ServerCountry.qml`, and `SystemTrayNotificationHandler.cpp` are expected to explicitly use assts.

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
