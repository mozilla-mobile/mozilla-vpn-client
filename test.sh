cmake -H/Users/basti/Code/mozilla-vpn-client \
-DCMAKE_FIND_ROOT_PATH=/Users/basti/Code/mozilla-vpn-client/android/client/.cxx/cmake/debug/prefab/x86/prefab:/Users/basti/Code/qt/6.2.4/android_x86/ \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_TOOLCHAIN_FILE=/Users/basti/Code/qt/6.2.4/android_x86/lib/cmake/Qt6/qt.toolchain.cmake \
-DANDROID_ABI=x86 \
-DANDROID_NDK=/Users/basti/Library/Android/sdk/ndk/23.1.7779620 \
-DANDROID_PLATFORM=android-24 \
-DCMAKE_ANDROID_ARCH_ABI=x86 \
-DCMAKE_ANDROID_NDK=/Users/basti/Library/Android/sdk/ndk/23.1.7779620 \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=/Users/basti/Code/mozilla-vpn-client/android/client/build/intermediates/cmake/debug/obj/x86 \
-DCMAKE_MAKE_PROGRAM=/opt/homebrew/bin/ninja \
-DCMAKE_SYSTEM_NAME=Android \
-DCMAKE_SYSTEM_VERSION=24 \
-DCMAKE_PREFIX_PATH=/Users/basti/Code/qt/6.2.4/android_x86/lib/cmake \
-B/Users/basti/Code/mozilla-vpn-client/android/client/.cxx/cmake/debug/x86 \
-DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=BOTH \
-GNinja 


