#ifndef SERVERSFETCHER_H
#define SERVERSFETCHER_H

#include <QObject>

class MozillaVPN;

class ServersFetcher final : public QObject
{
    Q_OBJECT

public:
    ServersFetcher(QObject *parent) : QObject(parent) {}

    void run(MozillaVPN *vpn);

signals:
    void completed(const QByteArray& serverData);
};

#endif // SERVERSFETCHER_H
