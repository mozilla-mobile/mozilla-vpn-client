#ifndef TESTDEVICE_H
#define TESTDEVICE_H

#include <QObject>

class TestDevice : public QObject
{
    Q_OBJECT

private slots:
    void basic();

    void fromJson_data();
    void fromJson();
};

#endif // TESTDEVICE_H
