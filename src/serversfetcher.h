#ifndef SERVERSFETCHER_H
#define SERVERSFETCHER_H

#include <QObject>

class MozillaVPN;
class ServerData;

class ServersFetcher : public QObject
{
    Q_OBJECT

public:
    ServersFetcher(QObject *parent) : QObject(parent) {}

    void run(MozillaVPN *vpn);

signals:
    void completed(ServerData *servers);
};

#endif // SERVERSFETCHER_H
