## To build and run the tests

* From a clean project at the root level
* `./scripts/utils/import_languages.py`
* `qmake CONFIG+=QMLTEST`
* `make -j XX` (number of procs you have)
* `./tests/qml/qml_tests`

## General notes, tips, etc.


You don't need to rerun `make` inbetween changes to `tst_*.qml` files.

I needed to make a property alias at a top level object to reference more nested things in the tests. There may be a better way where we're not cluttering up production code because of testing needs.

#### Setup order

The order in which things are setup is important.

The SettingsHolder is made in `main.cpp` so that there is an instance available for everything else to use.

Ths SettingsHolder must be instantiated for the FeatureList to be able to initialize, and the FeatureList must be initialized before WhatsNewModel can be created.

#### QML imports / organization

It can be a struggle to get the thing you want to import imported.
Don't struggle alone, ask for help.
Hopefully it's just figuring out the right combination of:
  * the .qrc files under `/src/ui`
  * the `qmldir`s
  * the RESOURCSE in `qml.pro`
  * the engine imports in `main.cpp`

Use `export QML_IMPORT_TRACE=1` before running your tests to get extra helpful informaiton.

As an example, under `/src/ui` there is a `ui.qrc`. That file lists the prefix as `ui` and points to a qmldir file `src/ui/qmldir`. The qmldir file lists `Main` and points to `src/ui/main.qml`. In `qml.pro` I have `...src/ui.qrc` as a `RESOURCE`; in `main.cpp` I import `engine->addImportPath("qrc:///");` because that pulls in the qmldir `src/ui/qmldir`. Lastly I can then do `import ui 0.1` in my `test_mainWindow.qml` and I can make a `Main` window.
