Building the for the first time can be a bit daunting.

We strongly recommend first building for the platform you are on. For example if you're on a windows machine, make a windows build; a mac, make a mac build etc.

When building for android and iOS you will be cross-compiling (building for a different platform than the machine you're on) which can make things a little trickier.

To try and provide a smooth dev experience we focus on ensuring that a minimal set of workflows are always working. There's nothing stopping you from using your preferred tools and workflows, but you are in the wild. Specifically:

* Pre-requisites:
  * Qt
  * All builds are done using cmake
  * Wherever possible, dependencies are managed using conda
  * Getting dependencies installed is discussed in each platform page
* [Linux app](./linux.md)
   * Build on Linux
   * Develop in VS Code
* [Windows app](./windows.md)
   * Build on Windows
   * Develop in Visual Studio
* [Mac app](./macos.md)
   * Build on Windows
   * Develop in VS Code or XCode
* [Android app](./android.md)
   * Build on Linux, Mac
   * Develop in VS Code
* [iOS app](./ios.md)
   * Build on Mac
   * Develop in VS Code or XCode
   * XCode required to build

If you want to submit a pull-request, please, install the clang format
pre-commit hook: `./scripts/git-pre-commit-format install`
