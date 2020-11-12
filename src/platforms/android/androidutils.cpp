/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "androidutils.h"

#include <jni.h>
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>


/**
 *  Gets the Device
 */
QString AndroidUtils::GetDeviceName(){
    QAndroidJniEnvironment env;
    jclass BUILD = env->FindClass("android/os/Build");
    jfieldID model = env->GetStaticFieldID(BUILD,"MODEL","Ljava/lang/String;");
    jstring value = (jstring) env->GetStaticObjectField(BUILD,model);
    if(!value){
        return QString("Android Device");
    }
    const char* buffer = env->GetStringUTFChars(value,nullptr);
    if(!buffer){
        return QString("Android Device");
    }
    QString res = QString(buffer);
    env->ReleaseStringUTFChars(value,buffer);
    return res;
};
