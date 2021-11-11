#### General notes, tips, etc.

* See the github workflow for steps to build, not much to it, just make sure you have the right qmake call.
* You don't need to rerun `make` inbetween changes to `tst_*.qml` files.
* It can be a struggle to get the thing you want to import imported. Don't struggle alone, ask for help. Hopefully it's
  just figuring out the right combination of:
  * the .qrc files under `/src/ui`
  * the `qmldir`s
  * the RESOURCSE in `qml.pro`
  * the engine imports in `main.cpp`
* I have made property aliases at top level of objects to reference more nested things in the tests. There may be a
  better way where we're not cluttering up production code because of testing needs. If so, let me know / make a PR.
