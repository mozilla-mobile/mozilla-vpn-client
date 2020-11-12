#ifndef ANDROIDUTILS_H
#define ANDROIDUTILS_H

#include <QAndroidBinder>
#include <QAndroidIntent>
#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>
#include <QtAndroid>

class AndroidUtils
{
public:
    static QString GetDeviceName();
};

#endif // ANDROIDUTILS_H
