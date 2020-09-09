#!/bin/bash

printv() {
  if [ -t 1 ]; then
    NCOLORS=$(tput colors)

    if test -n "$NCOLORS" && test "$NCOLORS" -ge 8; then
      NORMAL="$(tput sgr0)"
      RED="$(tput setaf 1)"
      GREEN="$(tput setaf 2)"
      YELLOW="$(tput setaf 3)"
    fi
  fi

  if [[ $2 = 'G' ]]; then
    # shellcheck disable=SC2086
    echo $1 -e "${GREEN}$3${NORMAL}"
  elif [[ $2 = 'Y' ]]; then
    # shellcheck disable=SC2086
    echo $1 -e "${YELLOW}$3${NORMAL}"
  elif [[ $2 = 'N' ]]; then
    # shellcheck disable=SC2086
    echo $1 -e "$3"
  else
    # shellcheck disable=SC2086
    echo $1 -e "${RED}$3${NORMAL}"
  fi
}

print() {
  printv '' "$1" "$2"
}

printn() {
  printv "-n" "$1" "$2"
}

error() {
  printv '' R "$1"
}

die() {
  error "$1"
  exit 1
}

print N "This script compiles MozillaVPN for linux using a static Qt5 build"
print N ""

if [ "$1" = "" ] || [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
  print G "Usage:"
  print N "\t$0 <destination_folder>"
  print N ""
  print G "Config variables:"
  print N "\tQTBIN=</path/of/the/qt/bin/folder>"
  print N ""
  exit 0
fi

if ! [ -d "src" ] || ! [ -d "linux" ]; then
  die "This script must be executed at the root of the repository."
fi

if [ "$QTBIN" ]; then
  QMAKE=$QTBIN/qmake
else
  QMAKE=qmake
fi

$QMAKE -v &>/dev/null || die "qmake doesn't exist or it fails"

printn Y "Cleaning the folder... "
make distclean &>/dev/null;
print G "done."

print Y "Configuring the build (qmake)..."
$QMAKE \
  CONFIG+=static \
  QTPLUGIN+=qsvg \
  QTPLUGIN.imageformats+=png \
  PREFIX=$1 || die "Compilation failed"

print Y "Compiling..."
make -j8
print G "Compilation completed!"

print Y "Installation..."
make install
print G "Installation completed!"
