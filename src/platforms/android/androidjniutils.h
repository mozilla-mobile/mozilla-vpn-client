#ifndef ANDROIDJNIUTILS_H
#define ANDROIDJNIUTILS_H


#include <QDebug>
#include <QRandomGenerator>
#include <QAndroidJniObject>
#include <QAndroidIntent>
#include <QtAndroid>
#include <QtAndroid>
#include <QDebug>

class androidJNIUtils
{
public:

    static void callFromJava(JNIEnv *env, jobject /*thiz*/, jstring value);
    static void startActivityForResult(JNIEnv *env, jobject /*thiz*/, jobject intent);
    static jobject getActivity(JNIEnv *env, jobject /*thiz*/);
    static void init();
};

#endif // ANDROIDJNIUTILS_H
