#!/bin/bash

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


pushd vcs
mkdir homebrew && curl -L https://github.com/Homebrew/brew/tarball/master | tar xz --strip 1 -C homebrew
export PATH=$PWD/homebrew/bin:$PATH

mkdir qt_dist
mkdir artifacts

brew install cmake
brew install ninja

QT_SOURCE_DIR=$(find $MOZ_FETCHES_DIR -maxdepth 1 -type d -name 'qt-everywhere-src-*' | head -1)
echo "Building $(basename $QT_SOURCE_DIR)"

cd $QT_SOURCE_DIR

bash ./configure \
  $* \
  --prefix=$PREFIX \
  -opensource \
  -confirm-license \
  -release \
  -static \
  -strip \
  -silent \
  -nomake tests \
  -make libs \
  -no-feature-dynamicgl \
  -no-feature-sql-odbc \
  -no-feature-pixeltool \
  -no-feature-qdbus \
  -no-feature-qtattributionsscanner \
  -no-feature-qtdiag \
  -no-feature-qtplugininfo \
  -no-feature-pixeltool \
  -no-feature-distancefieldgenerator \
  -no-feature-designer \
  -no-feature-assistant \
  -no-feature-style-fusion \
  -no-feature-style-windows \
  -no-feature-style-windowsvista \
  -no-feature-quickcontrols2-material \
  -no-feature-quickcontrols2-macos \
  -no-feature-quickcontrols2-imagine \
  -no-feature-quickcontrols2-ios \
  -no-feature-quickcontrols2-universal \
  -no-feature-quicktemplates2-calendar \
  -no-feature-quicktemplates2-hover \
  -no-feature-quicktemplates2-multitouch \
  -no-feature-qml-xml-http-request \
  -no-feature-qml-network \
  -no-feature-tiff \
  -no-feature-webp \
  -no-feature-cups \
  -no-feature-style-fusion \
  -no-feature-style-mac \
  -no-feature-style-windows \
  -no-feature-textmarkdownwriter \
  -no-feature-cssparser \
  -no-feature-qmake \
  -no-feature-itemmodeltester \
  -no-feature-quick-sprite \
  -no-feature-quick-tableview \
  -no-feature-quick-treeview \
  -no-feature-sql-sqlite \
  -no-feature-sql \
  -no-feature-textodfwriter \
  -no-feature-networklistmanager \
  -no-feature-style-stylesheet \
  -no-feature-itemviews \
  -no-feature-treewidget \
  -no-feature-listwidget \
  -no-feature-tablewidget \
  -no-feature-datetimeedit \
  -no-feature-stackedwidget \
  -no-feature-textbrowser \
  -no-feature-splashscreen \
  -no-feature-splitter \
  -no-feature-widgettextcontrol \
  -no-feature-label \
  -no-feature-formlayout \
  -no-feature-lcdnumber \
  -no-feature-menu \
  -no-feature-lineedit \
  -no-feature-radiobutton \
  -no-feature-spinbox \
  -no-feature-tabbar \
  -no-feature-tabwidget \
  -no-feature-combobox \
  -no-feature-fontcombobox \
  -no-feature-checkbox \
  -no-feature-pushbutton \
  -no-feature-toolbutton \
  -no-feature-toolbar \
  -no-feature-toolbox \
  -no-feature-groupbox \
  -no-feature-buttongroup \
  -no-feature-mainwindow \
  -no-feature-dockwidget \
  -no-feature-mdiarea \
  -no-feature-resizehandler \
  -no-feature-statusbar \
  -no-feature-menubar \
  -no-feature-contextmenu \
  -no-feature-progressbar \
  -no-feature-abstractslider \
  -no-feature-slider \
  -no-feature-scrollbar \
  -no-feature-dial \
  -no-feature-scrollarea \
  -no-feature-scroller \
  -no-feature-graphicsview \
  -no-feature-graphicseffect \
  -no-feature-textedit \
  -no-feature-syntaxhighlighter \
  -no-feature-rubberband \
  -no-feature-tooltip \
  -no-feature-statustip \
  -no-feature-sizegrip \
  -no-feature-calendarwidget \
  -no-feature-keysequenceedit \
  -no-feature-dialog \
  -no-feature-dialogbuttonbox \
  -no-feature-messagebox \
  -no-feature-colordialog \
  -no-feature-filedialog \
  -no-feature-fontdialog \
  -no-feature-progressdialog \
  -no-feature-inputdialog \
  -no-feature-errormessage \
  -no-feature-wizard \
  -no-feature-listview \
  -no-feature-tableview \
  -no-feature-treeview \
  -no-feature-datawidgetmapper \
  -no-feature-columnview \
  -no-feature-completer \
  -no-feature-fscompleter \
  -no-feature-undoview \
  -no-feature-dbus \
  -no-feature-xml \
  -skip qt3d  \
  -skip qtdoc \
  -skip qtgrpc \
  -skip qtconnectivity \
  -skip qtquickeffectmaker \
  -skip qtquicktimeline \
  -skip qtwebengine  \
  -skip qtlocation \
  -skip qtmultimedia  \
  -skip qtserialport  \
  -skip qtsensors  \
  -skip qtgamepad  \
  -skip qtgraphs \
  -skip qtandroidextras  \
  -skip qtquick3dphysics \
  -skip qtactiveqt  \
  -skip qtcharts  \
  -skip qtcoap  \
  -skip qtdatavis3d  \
  -skip qtgrpc  \
  -skip qtremoteobjects  \
  -skip qtlottie  \
  -skip qtmqtt  \
  -skip qtopcua  \
  -skip qtpositioning  \
  -skip qtquick3d  \
  -skip qtscxml  \
  -skip qtserialbus  \
  -skip qtserialport  \
  -skip qtspeech  \
  -skip qtvirtualkeyboard  \
  -skip qtweb \
  -skip qtwayland  \
  -qt-doubleconversion \
  -qt-libpng \
  -qt-zlib \
  -qt-pcre \
  -qt-freetype \
  -feature-imageformat_png \
  -feature-optimize_full \
  -appstore-compliant \
  -feature-texthtmlparser \
  -feature-xml \
  -- \
  -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'

cmake --build . --parallel 
cmake --install . 



















echo "Creating Qt dist artifact"
mkdir -p ../../public/build
zip -qr ../../public/build/qt6_mac.zip qt_dist/*
