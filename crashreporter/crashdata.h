#ifndef CRASHDATA_H
#define CRASHDATA_H

#include <QObject>
/**
 * @brief A base class for platform specific crash data sent to the handler.
 * Used to pass through non-platform specific code polymorphically.
 */
class CrashData : public QObject
{
    Q_OBJECT
public:
    explicit CrashData(QObject *parent = nullptr);

signals:

};

#endif // CRASHDATA_H
