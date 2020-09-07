#ifndef WGQUICKPROCESS_H
#define WGQUICKPROCESS_H

#include <QObject>
#include <QTemporaryDir>

class WgQuickProcess : public QObject
{
    Q_OBJECT
public:
    enum Op {
        Up,
        Down,
    };

    explicit WgQuickProcess(Op op);

    void run(const QString &privateKey,
             const QString &deviceIpv4Address,
             const QString &deviceIpv6Address,
             const QString &serverIpv4Gateway,
             const QString &serverPublicKey,
             const QString &serverIpv4AddrIn,
             int serverPort);

signals:
    void failed();
    void succeeded();

private:
    Op m_op;

    QTemporaryDir tmpDir;
};

#endif // WGQUICKPROCESS_H
