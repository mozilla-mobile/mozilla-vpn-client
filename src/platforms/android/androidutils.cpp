#include "androidutils.h"


#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <jni.h>
#include <QAndroidServiceConnection>
#include <QDebug>
#include <QtAndroid>

/**
 *  Gets the Device
 */
QString AndroidUtils::GetDeviceName(){
    QAndroidJniEnvironment env;
    jclass BUILD = env->FindClass("android/os/Build");
    jfieldID model = env->GetStaticFieldID(BUILD,"MODEL","Ljava/lang/String;");
    jstring value = (jstring) env->GetStaticObjectField(BUILD,model);

    const char* buffer = env->GetStringUTFChars(value,nullptr);
    QString res = QString(buffer);
    env->ReleaseStringUTFChars(value,buffer);
    return res;
};
