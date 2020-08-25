#ifndef WGQUICKPROCESS_H
#define WGQUICKPROCESS_H

#include <QObject>
#include <QTemporaryDir>

class Server;
class Device;
class Keys;

class WgQuickProcess : public QObject
{
    Q_OBJECT
public:
    enum Op {
        Up,
        Down,
    };

    explicit WgQuickProcess(Op op);

    void Run(const Server &server, const Device *device, const Keys *keys);

    static bool checkDependencies();

signals:
    void failed();
    void succeeded();

private:
    Op m_op;

    QTemporaryDir tmpDir;
};

#endif // WGQUICKPROCESS_H
