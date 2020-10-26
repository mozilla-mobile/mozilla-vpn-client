#include "androidjniutils.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QAndroidJniObject>
#include <QAndroidIntent>
#include <QAndroidJniEnvironment>
#include <QtAndroid>
#include <QtAndroid>
#include <QDebug>
#include <android/log.h>

#define TAG "JNI_UTILS"
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,    TAG, __VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,       TAG, __VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,      TAG, __VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,       TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,      TAG, __VA_ARGS__)





void androidJNIUtils::callFromJava(JNIEnv *env, jobject /*thiz*/, jstring value)
{
    qDebug() << env->GetStringUTFChars(value, nullptr);
    LOGD("%s",env->GetStringUTFChars(value, nullptr));

}


void androidJNIUtils::startActivityForResult(JNIEnv *env, jobject /*thiz*/, jobject intent)
{
    // Send the thing
    Q_UNUSED(env);
   LOGD("Trying to request VPN Permission");
    QtAndroid::startActivity(intent,123, [](int a, int b, const QAndroidJniObject& c){
        Q_UNUSED(a);
        Q_UNUSED(b);
        Q_UNUSED(c);
        LOGD("GOT INTENT RESULT");
    });

    LOGD("Intent was dispatched");
    return;
}

void androidJNIUtils::init(){
    JNINativeMethod methods[] {
        {"startActivityForResult", "(Landroid/content/Intent;)V", reinterpret_cast<void *>(startActivityForResult)}};

    QAndroidJniObject javaClass("com/mozilla/vpn/VPNService");
    QAndroidJniEnvironment env;
    jclass objectClass = env->GetObjectClass(javaClass.object<jobject>());
    env->RegisterNatives(objectClass,
                         methods,
                         sizeof(methods) / sizeof(methods[0]));
    env->DeleteLocalRef(objectClass);
}
