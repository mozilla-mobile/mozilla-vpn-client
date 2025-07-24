//go:build android
// +build android

package main

/*
#include <jni.h>

// Declare JNI helper functions since CGO can't resolve env-> calls directly
static const char* myGetStringUTFChars(JNIEnv* env, jstring str, jboolean* isCopy) {
    return (*env)->GetStringUTFChars(env, str, isCopy);
}
static void myReleaseStringUTFChars(JNIEnv* env, jstring str, const char* chars) {
    (*env)->ReleaseStringUTFChars(env, str, chars);
}
static jstring myNewStringUTF(JNIEnv* env, const char* bytes) {
    return (*env)->NewStringUTF(env, bytes);
}
*/
import "C"

//export Java_org_mozilla_guardian_tunnel_WireGuardGo_wgTurnOn
func Java_org_mozilla_guardian_tunnel_WireGuardGo_wgTurnOn(env *C.JNIEnv, clazz C.jclass, jInterfaceName C.jstring, tunFd C.jint, jSettings C.jstring) C.jint {
    interfaceNameC := C.myGetStringUTFChars(env, jInterfaceName, nil)
    settingsC := C.myGetStringUTFChars(env, jSettings, nil)

    interfaceName := C.GoString(interfaceNameC)
    settings := C.GoString(settingsC)

    ret := wgTurnOn(interfaceName, int32(tunFd), settings)

    C.myReleaseStringUTFChars(env, jInterfaceName, interfaceNameC)
    C.myReleaseStringUTFChars(env, jSettings, settingsC)

    return C.jint(ret)
}

//export Java_org_mozilla_guardian_tunnel_WireGuardGo_wgTurnOff
func Java_org_mozilla_guardian_tunnel_WireGuardGo_wgTurnOff(env *C.JNIEnv, clazz C.jclass, handle C.jint) {
    wgTurnOff(int32(handle))
}

//export Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetSocketV4
func Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetSocketV4(env *C.JNIEnv, clazz C.jclass, handle C.jint) C.jint {
    return C.jint(wgGetSocketV4(int32(handle)))
}

//export Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetSocketV6
func Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetSocketV6(env *C.JNIEnv, clazz C.jclass, handle C.jint) C.jint {
    return C.jint(wgGetSocketV6(int32(handle)))
}

//export Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetConfig
func Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetConfig(env *C.JNIEnv, clazz C.jclass, handle C.jint) C.jstring {
    config := wgGetConfig(int32(handle))
    if config == nil {
        return (C.jstring)(0)
    }
    return C.myNewStringUTF(env, config)
}

//export Java_org_mozilla_guardian_tunnel_WireGuardGo_wgVersion
func Java_org_mozilla_guardian_tunnel_WireGuardGo_wgVersion(env *C.JNIEnv, clazz C.jclass) C.jstring {
    version := wgVersion()
    return C.myNewStringUTF(env, version)
}

//export Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetLatestHandle
func Java_org_mozilla_guardian_tunnel_WireGuardGo_wgGetLatestHandle(env *C.JNIEnv, clazz C.jclass) C.jint {
    return C.jint(wgGetLatestHandle())
}
