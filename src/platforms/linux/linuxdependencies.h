#ifndef LINUXDEPENDENCIES_H
#define LINUXDEPENDENCIES_H

#include <QObject>

class LinuxDependencies
{
public:
    static bool checkDependencies();

private:
    LinuxDependencies() = default;
    ~LinuxDependencies() = default;

    Q_DISABLE_COPY(LinuxDependencies)
};

#endif // LINUXDEPENDENCIES_H
