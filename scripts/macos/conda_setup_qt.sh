#!/bin/bash

BASE_PREFIX=$(conda info --base)
if [[ "$CONDA_PREFIX" == "$BASE_PREFIX" ]]; then
    echo "Please run this script inside a non-base conda environment."
    exit 1
fi

if [[ "$(uname)" == "Darwin" ]]; then
    HOST_TARGET="mac desktop ${QT_VERSION}"
    HOST="mac"
    HOST_FOLDER_NAME="macos"
else
    echo "Unsupported operating system. Please run this script on or macOS."
    exit 1
fi

export QT_DIR=$CONDA_PREFIX/Qt
# QT_Host Tools
python -m aqt install-qt --outputdir $QT_DIR $HOST_TARGET
# QT Android Tools
python -m aqt install-qt --outputdir $QT_DIR $HOST ios ${QT_VERSION} -m all 

echo "$QT_DIR/$QT_VERSION/$ANDROID_ARCH/bin/qt-cmake"

## Generate activation/deactivation scripts, so stuff stays portable.

# Set vars on activation of the env
mkdir -p $CONDA_PREFIX/etc/conda/activate.d/
mkdir -p $CONDA_PREFIX/etc/conda/deactivate.d/
cat <<EOF > $CONDA_PREFIX/etc/conda/activate.d/vpn_ios_qt.sh
#!/bin/bash
export QT_HOST_PATH=\$CONDA_PREFIX/Qt/$QT_VERSION/$HOST_FOLDER_NAME
EOF
chmod +x $CONDA_PREFIX/etc/conda/activate.d/vpn_ios_qt.sh
## Remove Vars on deactivation
cat <<EOF > $CONDA_PREFIX/etc/conda/deactivate.d/vpn_ios_qt.sh
#!/bin/bash
unset QT_HOST_PATH
EOF
chmod +x $CONDA_PREFIX/etc/conda/deactivate.d/vpn_ios_qt.sh

## Add a Helper script to call the right qt-cmake

cat <<EOF > $CONDA_PREFIX/bin/qt-cmake
#!/bin/bash
$CONDA_PREFIX/Qt/$QT_VERSION/ios/bin/qt-cmake -DQT_HOST_PATH=$QT_HOST_PATH -GXcode "\$@"
EOF
chmod +x $CONDA_PREFIX/etc/conda/deactivate.d/vpn_ios_qt.sh



echo "finished setup $QT_VERSION QT to compile for MacOS + iOS "
echo "please re-enable the VPN env to apply changes."


