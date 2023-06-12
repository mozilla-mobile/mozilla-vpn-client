#!/bin/bash

BASE_PREFIX=$(conda info --base)
if [[ "$CONDA_PREFIX" == "$BASE_PREFIX" ]]; then
    echo "Please run this script inside a non-base conda environment."
    exit 1
fi

if [[ "$(uname)" == "Linux" ]]; then
    COMMANDLINE_TOOLS_URL="https://dl.google.com/android/repository/commandlinetools-linux-9477386_latest.zip"
elif [[ "$(uname)" == "Darwin" ]]; then
    COMMANDLINE_TOOLS_URL="https://dl.google.com/android/repository/commandlinetools-mac-9477386_latest.zip"
else
    echo "Unsupported operating system. Please run this script on Linux or macOS."
    exit 1
fi

cp android_sdk.txt $CONDA_PREFIX/

cd $CONDA_PREFIX
mkdir -p bin # Might not exist in an empty env.
# Download Commandline tools, smylink them to the conda path.
wget -O commandline_tools.zip $COMMANDLINE_TOOLS_URL
unzip commandline_tools.zip
ln -s $CONDA_PREFIX/cmdline-tools/bin/sdkmanager $CONDA_PREFIX/bin/sdkmanager
ln -s $CONDA_PREFIX/cmdline-tools/bin/screenshot2 $CONDA_PREFIX/bin/screenshot2
ln -s $CONDA_PREFIX/cmdline-tools/bin/retrace $CONDA_PREFIX/bin/retrace
ln -s $CONDA_PREFIX/cmdline-tools/bin/profgen $CONDA_PREFIX/bin/profgen
ln -s $CONDA_PREFIX/cmdline-tools/bin/avdmanager $CONDA_PREFIX/bin/avdmanager
ln -s $CONDA_PREFIX/cmdline-tools/bin/apkanalyzer $CONDA_PREFIX/bin/apkanalyzer
rm commandline_tools.zip

export ANDROID_HOME=$CONDA_PREFIX/android_home

echo y | sdkmanager --install $(cat android_sdk.txt) --sdk_root=$ANDROID_HOME


installed_things=$(sdkmanager --list_installed --sdk_root=$ANDROID_HOME | grep "ndk")
if [[ $installed_things =~ ndk/([0-9]+\.[0-9]+\.[0-9]+) ]]; then
  ndk_path=${BASH_REMATCH[1]}
fi

# Export ADB to env.
ln -s $CONDA_PREFIX/android_home/platform-tools/adb $CONDA_PREFIX/bin/adb

sdkmanager --list_installed --sdk_root=$ANDROID_HOME
echo "Android SDK installed in env $CONDA_PREFIX, please restart conda env to have changes applied"

## Generate activation/deactivation scripts, so stuff stays portable.

mkdir -p $CONDA_PREFIX/etc/conda/activate.d/
mkdir -p $CONDA_PREFIX/etc/conda/deactivate.d/
cat <<EOF > $CONDA_PREFIX/etc/conda/activate.d/vpn_android_sdk.sh
#!/bin/bash
export ANDROID_SDK_ROOT=\$CONDA_PREFIX/android_home
export ANDROID_HOME=\$CONDA_PREFIX/android_home
export ANDROID_NDK_HOME=\$CONDA_PREFIX/android_home/ndk/$ndk_path
export ANDROID_NDK_ROOT=\$CONDA_PREFIX/android_home/ndk/$ndk_path
EOF
chmod +x $CONDA_PREFIX/etc/conda/activate.d/vpn_android_sdk.sh


cat <<EOF > $CONDA_PREFIX/etc/conda/deactivate.d/vpn_android_sdk.sh
#!/bin/bash
unset ANDROID_SDK_ROOT
unset ANDROID_HOME
unset ANDROID_NDK_HOME
unset ANDROID_NDK_ROOT
EOF
chmod +x $CONDA_PREFIX/etc/conda/deactivate.d/vpn_android_sdk.sh
