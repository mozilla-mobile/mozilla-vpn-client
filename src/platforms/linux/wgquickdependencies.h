#ifndef WGQUICKDEPENDENCIES_H
#define WGQUICKDEPENDENCIES_H

#include <QObject>

class WgQuickDependencies
{
public:
    static bool checkDependencies();

private:
    WgQuickDependencies() = default;
    ~WgQuickDependencies() = default;

    Q_DISABLE_COPY(WgQuickDependencies)
};

#endif // WGQUICKDEPENDENCIES_H
