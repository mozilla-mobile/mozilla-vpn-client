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

/**
 * @brief Starts the Given intent in Context of the QTActivity
 * @param env
 * @param intent
 */
void androidJNIUtils::startActivityForResult(JNIEnv *env, jobject /*thiz*/, jobject intent)
{
    Q_UNUSED(env);
    QtAndroid::startActivity(intent,123, [](int a, int b, const QAndroidJniObject& c){
        // TODO: Automaticly make the Android Controller Retry if the result is positive.
        // TODO: Maybe move into AndroidController?
        Q_UNUSED(a);
        Q_UNUSED(b);
        Q_UNUSED(c);
    });
    return;
}
/**
 * @brief Registers the native JNIUtils functions in the Java Env.
 */
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
