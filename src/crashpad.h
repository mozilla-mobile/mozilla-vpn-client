#ifndef CRASHPAD_H
#define CRASHPAD_H

#include <QObject>

class Crashpad : public QObject
{
    Q_OBJECT
public:
    Crashpad() = default;
    virtual ~Crashpad() = default;
    virtual bool start() = 0;
};

#endif // CRASHPAD_H
