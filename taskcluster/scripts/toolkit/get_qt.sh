#!/bin/sh
set -x
set -e

$OLD= $PWD
# This Pulls QT into /fetches
# TODO: Make this an ENV
cd $1
wget -q https://download.qt.io/archive/qt/6.2/6.2.3/single/qt-everywhere-src-6.2.3.zip || true
unzip -o -qq qt-everywhere-src-6.2.3.zip|| true

cd $OLD
