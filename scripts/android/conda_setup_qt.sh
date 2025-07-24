#!/bin/bash
set -e

BASE_PREFIX=$(conda info --base)
if [[ "$CONDA_PREFIX" == "$BASE_PREFIX" ]]; then
    echo "Please run this script inside a non-base conda environment."
    exit 1
fi

# Extract major, minor, and patch version numbers from QT_VERSION
QT_MAJOR=$(echo "$QT_VERSION" | cut -d. -f1)
QT_MINOR=$(echo "$QT_VERSION" | cut -d. -f2)
QT_PATCH=$(echo "$QT_VERSION" | cut -d. -f3)

if [[ "$(uname)" == "Linux" ]]; then
    HOST="linux"
    # Ensure QT_MINOR is parsed as a number before comparison
    QT_MINOR_NUM=$((10#$QT_MINOR))
    if [[ "$QT_MINOR_NUM" -ge 8 ]]; then
        HOST_TARGET="linux desktop ${QT_VERSION} linux_gcc_64"
        HOST_FOLDER_NAME="linux_gcc_64"
    else
        HOST_TARGET="linux desktop ${QT_VERSION} gcc_64"
        HOST_FOLDER_NAME="gcc_64"
    fi
elif [[ "$(uname)" == "Darwin" ]]; then
    HOST_TARGET="mac desktop ${QT_VERSION}"
    HOST="mac"
    HOST_FOLDER_NAME="macos"
else
    echo "Unsupported operating system. Please run this script on Linux or macOS."
    exit 1
fi

# TODO: 
# let them define for what qt they want to compile.
if [[ -z "${ANDROID_ARCH}" ]]; then
    ANDROID_ARCH="android_arm64_v8a"
fi

export QT_DIR=$CONDA_PREFIX/Qt

# QT_Host Tools
echo "python -m aqt install-qt --outputdir $QT_DIR $HOST_TARGET"
python -m aqt install-qt --outputdir $QT_DIR $HOST_TARGET
for QT_HOST_DIR in $(find ${QT_DIR} -type d -name "${HOST_FOLDER_NAME}"); do
    find ${QT_HOST_DIR} -type f -name 'lib*.a' -delete
done


echo "python -m aqt install-qt --outputdir $QT_DIR $HOST android ${QT_VERSION} ${ANDROID_ARCH} -m all"
# QT Android Tools
if ! python -m aqt install-qt --outputdir $QT_DIR $HOST android ${QT_VERSION} ${ANDROID_ARCH} -m all; then
    echo "Whoops something went wrong. "
    echo "If no pri was found make sure your ANDROID_ARCH is one of:"
    python -m aqt list-qt $HOST android --arch $QT_VERSION
    exit 1
fi

echo "$QT_DIR/$QT_VERSION/$ANDROID_ARCH/bin/qt-cmake"
chmod +x $QT_DIR/$QT_VERSION/$ANDROID_ARCH/bin/qt-cmake


## Generate activation/deactivation scripts, so stuff stays portable.

# Set vars on activation of the env
mkdir -p $CONDA_PREFIX/etc/conda/activate.d/
mkdir -p $CONDA_PREFIX/etc/conda/deactivate.d/
cat <<EOF > $CONDA_PREFIX/etc/conda/activate.d/vpn_android_qt.sh
#!/bin/bash
export QT_HOST_PATH=\$CONDA_PREFIX/Qt/$QT_VERSION/$HOST_FOLDER_NAME
export QTPATH=\$CONDA_PREFIX/Qt/$QT_VERSION/$ANDROID_ARCH
EOF
chmod +x $CONDA_PREFIX/etc/conda/activate.d/vpn_android_qt.sh
## Remove Vars on deactivation
cat <<EOF > $CONDA_PREFIX/etc/conda/deactivate.d/vpn_android_qt.sh
#!/bin/bash
unset QT_HOST_PATH
unset QTPATH
EOF
chmod +x $CONDA_PREFIX/etc/conda/deactivate.d/vpn_android_qt.sh




echo "finished setup $QT_VERSION QT to compile for $ANDROID_ARCH"
echo "please re-enable the VPN env to apply changes."


