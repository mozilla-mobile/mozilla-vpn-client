#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
  if [[ "$1" ]]; then
    error "$1"
  else
    error Failed
  fi

  exit 1
}

print Y "Installing dependencies..."
apt-get update || die
apt-get install -y \
  wget \
  xz-utils \
  devscripts || die

printn Y "Creating /tmp/qt_ppa_final... "
rm -rf /tmp/qt_ppa_final || die
mkdir /tmp/qt_ppa_final || die
print G "done."

printn Y "Creating /tmp/qt_ppa... "
rm -rf /tmp/qt_ppa || die
mkdir /tmp/qt_ppa || die
cd /tmp/qt_ppa || die
print G "done."

magic() {
  NAME=$1
  FOLDER=$2
  ORIGURL=$3
  ORIG=$4
  DEBURL=$5
  DEB=$6

  if ! [[ -f $ORIG ]]; then
    print Y "Downloading the orig... "
    wget $ORIGURL || die
    tar xf $ORIG || die
  fi

  [[ -d $FOLDER ]] || die "$FOLDER doesn't exist."

  print Y "Downloading the deb for qt base... "
  wget $DEBURL || die
  tar xf $DEB || die
  rm -f $DEB || die
  print G "done."

  print Y "Patching debian files... "
  cat > tmp << EOF
$NAME (5.15.2-bionic1) bionic; urgency=low

  * Bionic build

 -- Andrea Marchesini <baku@mozilla.com>  $(date -R)

EOF

  cat debian/changelog >> tmp || die
  mv tmp debian/changelog || die
  print G "done."

  print Y "Installing dependencies... "

  BD=
  LIST=$(
  cat debian/control | while read LINE; do
    if [[ $(echo $LINE | grep "^Build-Depends:") ]]; then
      BD=1
      LINE=$(echo $LINE | cut -d: -f2)
    elif [[ $(echo $LINE | grep ":") ]]; then
      BD=
    fi

    if [[ $BD ]]; then
      echo $LINE | cut -d, -f1 | cut -d\[ -f1 | cut -d\( -f1 | grep -v g++-4.6
    fi
  done
  )

  echo $LIST
  apt-get install -y $LIST

  print Y "Configuring the source folder... "
  mv debian $FOLDER || die
  cd $FOLDER || die

  print Y "Creating the debian package... "
  debuild -S --no-sign || die

  dpkg-buildpackage -b -rfakeroot -us -uc   || die

  dpkg -i ../"$NAME"_*.deb

  print Y "Clean up... "
  cd .. || die
  rm -rf $FOLDER || die
  mv * /tmp/qt_ppa_final || die
  cp /tmp/qt_ppa_final/*orig.tar.xz . || die
  print G "done."
}

magic \
  qt515base \
  qtbase-everywhere-src-5.15.2 \
  https://launchpad.net/~mozillacorp/+archive/ubuntu/mozillavpn/+sourcefiles/qt515base/5.15.2-1basyskom4/qt515base_5.15.2.orig.tar.xz \
  qt515base_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515base/5.15.2-1basyskom4/qt515base_5.15.2-1basyskom4.debian.tar.xz \
  qt515base_5.15.2-1basyskom4.debian.tar.xz || die

magic \
  qt515xmlpatterns \
  qtxmlpatterns-everywhere-src-5.15.2 \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515xmlpatterns/5.15.2-1basyskom1/qt515xmlpatterns_5.15.2.orig.tar.xz \
  qt515xmlpatterns_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515xmlpatterns/5.15.2-1basyskom1/qt515xmlpatterns_5.15.2-1basyskom1.debian.tar.xz \
  qt515xmlpatterns_5.15.2-1basyskom1.debian.tar.xz || die

magic \
  qt515declarative \
  qtdeclarative-everywhere-src-5.15.2 \
  https://launchpad.net/~mozillacorp/+archive/ubuntu/mozillavpn/+sourcefiles/qt515declarative/5.15.2-1basyskom1/qt515declarative_5.15.2.orig.tar.xz \
  qt515declarative_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515declarative/5.15.2-1basyskom1/qt515declarative_5.15.2-1basyskom1.debian.tar.xz \
  qt515declarative_5.15.2-1basyskom1.debian.tar.xz || die

magic \
  qt515graphicaleffects \
  qtgraphicaleffects-everywhere-src-5.15.2 \
  https://launchpad.net/~mozillacorp/+archive/ubuntu/mozillavpn/+sourcefiles/qt515graphicaleffects/5.15.2-1basyskom1/qt515graphicaleffects_5.15.2.orig.tar.xz \
  qt515graphicaleffects_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515graphicaleffects/5.15.2-1basyskom1/qt515graphicaleffects_5.15.2-1basyskom1.debian.tar.xz \
  qt515graphicaleffects_5.15.2-1basyskom1.debian.tar.xz || die

magic \
  qt515imageformats \
  qtimageformats-everywhere-src-5.15.2 \
  https://launchpad.net/~mozillacorp/+archive/ubuntu/mozillavpn/+sourcefiles/qt515imageformats/5.15.2-1basyskom1/qt515imageformats_5.15.2.orig.tar.xz \
  qt515imageformats_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515imageformats/5.15.2-1basyskom2/qt515imageformats_5.15.2-1basyskom2.debian.tar.xz \
  qt515imageformats_5.15.2-1basyskom2.debian.tar.xz || die

magic \
  qt515networkauth-no-lgpl \
  qtnetworkauth-everywhere-src-5.15.2 \
  https://launchpad.net/~mozillacorp/+archive/ubuntu/mozillavpn/+sourcefiles/qt515networkauth-no-lgpl/5.15.2-1basyskom1/qt515networkauth-no-lgpl_5.15.2.orig.tar.xz \
  qt515networkauth-no-lgpl_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515networkauth-no-lgpl/5.15.2-1basyskom1/qt515networkauth-no-lgpl_5.15.2-1basyskom1.debian.tar.xz \
  qt515networkauth-no-lgpl_5.15.2-1basyskom1.debian.tar.xz || die

magic \
  qt515quickcontrols \
  qtquickcontrols-everywhere-src-5.15.2 \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515quickcontrols/5.15.2-1basyskom1/qt515quickcontrols_5.15.2.orig.tar.xz \
  qt515quickcontrols_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515quickcontrols/5.15.2-1basyskom1/qt515quickcontrols_5.15.2-1basyskom1.debian.tar.xz \
  qt515quickcontrols_5.15.2-1basyskom1.debian.tar.xz || die

magic \
  qt515quickcontrols2 \
  qtquickcontrols2-everywhere-src-5.15.2 \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515quickcontrols2/5.15.2-1basyskom1/qt515quickcontrols2_5.15.2.orig.tar.xz \
  qt515quickcontrols2_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515quickcontrols2/5.15.2-1basyskom1/qt515quickcontrols2_5.15.2-1basyskom1.debian.tar.xz \
  qt515quickcontrols2_5.15.2-1basyskom1.debian.tar.xz || die

magic \
  qt515svg \
  qtsvg-everywhere-src-5.15.2 \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515svg/5.15.2-1basyskom1/qt515svg_5.15.2.orig.tar.xz \
  qt515svg_5.15.2.orig.tar.xz \
  https://launchpad.net/~beineri/+archive/ubuntu/opt-qt-5.15.2-bionic/+sourcefiles/qt515svg/5.15.2-1basyskom1/qt515svg_5.15.2-1basyskom1.debian.tar.xz \
  qt515svg_5.15.2-1basyskom1.debian.tar.xz || die

print G "Now, upload all the files from /tmp/qt_ppa_final"
