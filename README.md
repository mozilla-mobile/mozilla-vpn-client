# Mozilla VPN

## Dependencies

MozillaVPN requires Qt5 (5.12 or higher)

### How to compile it for testing/developing

You can use QtCreator. Or if you like to compile in CLI, just do:

$ qmake
$ make -j4

On linux:
$ ./src/mozillavpn

On macos:
$ open ./src/MozillaVPN.app

### How to compile it in release mode for linux

Use ./scripts/linux_compile.sh

### How to compile it in release mode for MacOS

1. Install XCodeProj:
  $ [sudo] gem install xcodeproj
2. Update the submodules:
  $ git submodule init
  $ git submodule update --remote
3. Run the script:
  $ ./scripts/mac_compile.sh

