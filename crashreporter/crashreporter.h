#ifndef CRASHREPORTER_H
#define CRASHREPORTER_H

#include <QObject>

#include "crashdata.h"

class CrashData;

class CrashReporter : public QObject
{
    Q_OBJECT
public:
    explicit CrashReporter(QObject *parent = nullptr);
    virtual bool start() = 0;
    virtual void stop() {};
    virtual bool shouldPromptUser();
public slots:
    void crashReported(const CrashData &data);
signals:

};

#endif // CRASHREPORTER_H
